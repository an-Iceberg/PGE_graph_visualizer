#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <map>
#include <unordered_map>

enum mode
{
  MOVE,
  NODE,
  LINE,
  PATH
};

struct line
{
  int from;
  int to;
  int length;

  line(int from, int to, int length)
  {
    this->from = from;
    this->to = to;
    this->length = length;
  }
};

class PGE_graph_visualiser : public olc::PixelGameEngine
{
public:
  PGE_graph_visualiser() { sAppName = "PGE Graph Visualiser"; }

private:
  int UI_section_height = 100;
  int radius = 10;
  int selected_node = 0;
  int line_length = 1;
  int start = 0;
  int end = 0;
  bool graph_has_changed = false;
  mode mode = MOVE;
  std::vector<line> lines = {};
  std::map<int, olc::vi2d> nodes = {}; // The key serves as the ID of the node
  std::vector<int> path = {};

public:
  bool OnUserCreate() override { return true; }

  bool OnUserUpdate(float elapsed_time) override
  {
    if (IsFocused())
    {
      Clear(olc::BLACK);

      handle_mode_change_with_keys();
      handle_input();

      if (graph_has_changed) reset_graph();

      paint_lines();

      if (mode == PATH)
      {
        paint_start_and_end();
        paint_path();
      }

      paint_nodes();

      paint_UI();
    }

    if (GetKey(olc::D).bPressed)
    {
      std::cout << "Nodes:" << '\n';
      for (const auto& node : nodes) std::cout << node.first << ' ' << node.second << '\n';
      std::cout << '\n';
    }

    return true;
  }


  // ----------


private:
  void handle_mode_change_with_keys()
  {
    if (GetKey(olc::M).bPressed) mode = MOVE;
    else if (GetKey(olc::N).bPressed) mode = NODE;
    else if (GetKey(olc::L).bPressed) mode = LINE;
    else if (GetKey(olc::P).bPressed) mode = PATH;
  }

  void handle_input()
  {
    if (mode == MOVE)
    {
      // The node which the mouse is hovering over is being selected
      if (GetMouse(0).bPressed)
      {
        for (const auto& node : nodes) if (is_mouse_in_circle(node.second)) selected_node = node.first;
      }
      // Moving the node around
      else if (GetMouse(0).bHeld and selected_node != 0)
      {
        nodes[selected_node] = {GetMouseX(), GetMouseY()};

        if (nodes[selected_node].x < radius) nodes[selected_node].x = radius;
        if (nodes[selected_node].y < UI_section_height + radius) nodes[selected_node].y = UI_section_height + radius;
        if (nodes[selected_node].x > ScreenWidth() - radius) nodes[selected_node].x = ScreenWidth() - radius;
        if (nodes[selected_node].y > ScreenHeight() - radius) nodes[selected_node].y = ScreenHeight() - radius;
      }
      // Releasing the node from our iron grip
      else if (GetMouse(0).bReleased) selected_node = 0;
    }
    else if (mode == NODE)
    {
      // Create node on primary mouse click
      if (GetMouseY() > UI_section_height and GetMouse(0).bPressed)
      {
        // Only create a new node if it does not overlap with any existing one
        for (const auto& node : nodes)
        {
          // Giving the mouse a bit of a deadzone around it just to be safe
          // Not creating a node if the mouse overlaps with an existing node (hence the early return)
          if (do_circles_overlap(node.second, {GetMouseX() + 2, GetMouseY() + 2})) return;
        }

        // Creating a new node
        nodes[generate_node_ID()] = {GetMouseX(), GetMouseY()};

        graph_has_changed = true;
      }
      // Deleting a node and all lines coming/going from/to it
      else if (GetMouse(1).bPressed)
      {
        // Finding the node
        for (const auto& node : nodes)
        {
          if (do_circles_overlap(node.second, {GetMouseX(), GetMouseY()}))
          {
            // Deleting all lines associated with said node
            // Using an iterator because only an iterator allows one to delete an element
            for (std::vector<line>::iterator line = lines.begin(); line < lines.end(); line++)
            {
              if (line->from == node.first or line->to == node.first)
              {
                lines.erase(line);
                line--;
              }
            }

            // Deleting the node
            nodes.erase(node.first);
            graph_has_changed = true;
            break;
          }
        }
      }

      // If user presses delete or backspace they delete all nodes and lines
      if (GetKey(olc::BACK).bPressed or GetKey(olc::DEL).bPressed)
      {
        lines.clear();
        nodes.clear();
        graph_has_changed = true;
      }
    }
    else if (mode == LINE)
    {
      if (GetMouse(0).bPressed)
      {
        // Select a node but only if none have already been selected
        if (selected_node == 0)
        {
          bool ANodeHasBeenSelected = false;

          for (const auto& node : nodes)
          {
            if (not is_mouse_in_circle(node.second)) continue;

            selected_node = node.first;
            ANodeHasBeenSelected = true;
            break;
          }

          if (not ANodeHasBeenSelected) selected_node = 0;
        }
        // Create a new line but only if that line doesn't exist yet
        else
        {
          for (const auto& node : nodes)
          {
            if (not is_mouse_in_circle(node.second)) continue;

            // TODO: refactor this
            bool line_exists_already = false;

            // Only creating a new line if none exists yet in either direction
            for (const auto& aLine : lines)
            {
              // Ignore the line if it already exists
              if ((aLine.from == node.first and aLine.to == selected_node) or (aLine.from == selected_node and aLine.to == node.first))
              {
                line_exists_already = true;
                break;
              }
            }

            if (not line_exists_already)
            {
              lines.push_back(line(selected_node, node.first, line_length));
              graph_has_changed = true;
            }
          }

          selected_node = 0;
        }
      }
      // Delete a line with right click
      else if (GetMouse(1).bPressed)
      {
        // Only delete a line if a node has been selected
        if (selected_node != 0)
        {
          for (const auto& node : nodes)
          {
            if (not is_mouse_in_circle(node.second)) continue;

            // Using an iterator because it allows for element deletion
            for (std::vector<line>::iterator line = lines.begin(); line < lines.end(); line++)
            {
              if (not (line->from == selected_node and line->to == node.first)) continue;

              lines.erase(line);
              line--;
              graph_has_changed = true;
            }
          }

          selected_node = 0;
        }
      }

      if (GetKey(olc::RIGHT).bPressed)
      {
        increment_line_length();
      }
      else if (GetKey(olc::LEFT).bPressed)
      {
        decrement_line_length();
      }

      // If user presses delete or backspace they delete all lines
      if (GetKey(olc::BACK).bPressed or GetKey(olc::DEL).bPressed) lines.clear();
    }
    else if (mode == PATH)
    {
      // Select a node to be the start
      if (GetMouse(0).bPressed and GetMouseY() > UI_section_height)
      {
        bool new_node_has_been_selected = false;

        for (const auto& node : nodes)
        {
          if (is_mouse_in_circle(node.second))
          {
            start = node.first;
            new_node_has_been_selected = true;
            break;
          }
        }

        if (not new_node_has_been_selected) start = 0;

        graph_has_changed = true;
      }

      // Select a node to be the end
      if (GetMouse(1).bPressed and GetMouseY() > UI_section_height)
      {
        bool new_node_has_been_selected = false;

        for (const auto& node : nodes)
        {
          if (is_mouse_in_circle(node.second))
          {
            end = node.first;
            new_node_has_been_selected = true;
            break;
          }
        }

        if (not new_node_has_been_selected) end = 0;

        graph_has_changed = true;
      }

      // Calculate the path
      if (GetKey(olc::ENTER).bPressed)
      {
        // TODO: this 😓
      }

      // Clearing the start and end node
      if (GetKey(olc::BACK).bPressed or GetKey(olc::DEL).bPressed)
      {
        start = 0;
        end = 0;
      }
    }
  }

  void reset_graph()
  {
    path.clear();
    graph_has_changed = false;
  }

  void paint_UI()
  {
    // Draws a border around the UI section
    for (int i = 0; i < 6; i++) DrawRect(0 + i, 0 + i, ScreenWidth() - ((2 * i) + 1), UI_section_height - ((2 * i) + 1), olc::Pixel(128, 0, 255));
    FillRect(6, 6, ScreenWidth() - 12, UI_section_height - 12, olc::VERY_DARK_CYAN);

    DrawStringProp({10, 10}, "Mode:", olc::GREY, 2);
    DrawString({90, 10}, " M  N  L  P", olc::MAGENTA, 2);

    // Painting the mode selector
    // UI for MOVE
    if (mode == MOVE)
    {
      DrawString({91, 10}, "[ ]", olc::WHITE, 2);
      DrawStringProp({285, 10}, "- Move", olc::GREY, 2);

      if (selected_node == 0)
      {
        DrawStringProp({10, 29}, "Left Mouse: hold to move a node around", olc::GREY, 2);
        DrawStringProp({10, 29}, "Left Mouse:", olc::MAGENTA, 2);
      }
      else DrawStringProp({10, 29}, "Release to place node", olc::GREY, 2);
    }
    // UI for NODE
    else if (mode == NODE)
    {
      DrawString({139, 10}, "[ ]", olc::WHITE, 2);
      DrawStringProp({285, 10}, "- Node", olc::GREY, 2);

      DrawStringProp({10, 29}, "Left Mouse: create a node", olc::GREY, 2);
      DrawStringProp({10, 29}, "Left Mouse", olc::MAGENTA, 2);
      DrawStringProp({10, 48}, "Right Mouse: delete a node", olc::GREY, 2);
      DrawStringProp({10, 48}, "Right Mouse", olc::MAGENTA, 2);
      DrawStringProp({10, 67}, "Backspace/Delete: delete all nodes", olc::GREY, 2);
      DrawStringProp({10, 67}, "Backspace", olc::MAGENTA, 2);
      DrawStringProp({158, 67}, "Delete", olc::MAGENTA, 2);
    }
    // UI for LINE
    else if (mode == LINE)
    {
      DrawString({187, 10}, "[ ]", olc::WHITE, 2);
      DrawStringProp({285, 10}, "- Line", olc::GREY, 2);

      if (selected_node == 0)
      {
        DrawStringProp({10, 29}, "Left Mouse: select a node", olc::GREY, 2);
        DrawStringProp({10, 29}, "Left Mouse:", olc::MAGENTA, 2);
      }
      else
      {
        DrawStringProp({10, 29}, "Left Mouse: select another node to create a new line", olc::GREY, 2);
        DrawStringProp({10, 29}, "Left Mouse", olc::MAGENTA, 2);
        DrawStringProp({10, 48}, "Right Mouse: select another node to delete a line", olc::GREY, 2);
        DrawStringProp({10, 48}, "Right Mouse", olc::MAGENTA, 2);
      }

      DrawStringProp({10, 67}, "Backspace/Delete: delete all lines", olc::GREY, 2);
      DrawStringProp({10, 67}, "Backspace", olc::MAGENTA, 2);
      DrawStringProp({158, 67}, "Delete", olc::MAGENTA, 2);

      int distance = 450;
      // Adjust line length
      DrawStringProp({460, 67}, "Line length:", olc::GREY, 2);
      DrawString({600, 67}, "<" + (line_length < 10 ? '0' + std::to_string(line_length) : std::to_string(line_length)) + ">", olc::GREY, 2);
      DrawString({600, 67}, "<", olc::MAGENTA, 2);
      DrawString({648, 67}, ">", olc::MAGENTA, 2);

      // Hover on arrow keys for line length
      if (is_mouse_in_rect({598, 65}, {13, 17}))
      {
        DrawRect({598, 65}, {13, 17}, olc::GREY);
        if (GetMouse(0).bPressed) decrement_line_length();
      }
      else if (is_mouse_in_rect({648, 65}, {13, 17}))
      {
        DrawRect({648, 65}, {13, 17}, olc::GREY);
        if (GetMouse(0).bPressed) increment_line_length();
      }
    }
    // UI for PATH
    else if (mode == PATH)
    {
      DrawString({235, 10}, "[ ]", olc::WHITE, 2);
      DrawStringProp({285, 10}, "- Path", olc::GREY, 2);

      DrawStringProp({10, 29}, "Left Mouse: select a node to be the start", olc::GREY, 2);
      DrawStringProp({10, 29}, "Left Mouse", olc::MAGENTA, 2);
      DrawStringProp({10, 48}, "Right Mouse: select a node to be the end", olc::GREY, 2);
      DrawStringProp({10, 48}, "Right Mouse", olc::MAGENTA, 2);
      DrawStringProp({10, 67}, "Backspace/Delete: delete clear start and end", olc::GREY, 2);
      DrawStringProp({10, 67}, "Backspace", olc::MAGENTA, 2);
      DrawStringProp({158, 67}, "Delete", olc::MAGENTA, 2);

      DrawStringProp({590, 29}, "Enter: calculate shortest path from start to end", olc::GREY, 2);
      DrawStringProp({590, 29}, "Enter", olc::MAGENTA, 2);
    }

    // Hover on 'M'
    if (mode != MOVE and is_mouse_in_rect({103, 7}, {18, 19}))
    {
      DrawString({91, 10}, "[ ]", olc::GREY, 2);
      if (GetMouse(0).bPressed) mode = MOVE;
    }
    // Hover on 'N'
    else if (mode != NODE and is_mouse_in_rect({151, 7}, {18, 19}))
    {
      DrawString({139, 10}, "[ ]", olc::GREY, 2);
      if (GetMouse(0).bPressed) mode = NODE;
    }
    // Hover on 'L'
    else if (mode != LINE and is_mouse_in_rect({200, 7}, {18, 19}))
    {
      DrawString({187, 10}, "[ ]", olc::GREY, 2);
      if (GetMouse(0).bPressed) mode = LINE;
    }
    // Hover on 'P'
    else if (mode != PATH and is_mouse_in_rect({247, 7}, {18, 19}))
    {
      DrawString({235, 10}, "[ ]", olc::GREY, 2);
      if (GetMouse(0).bPressed) mode = PATH;
    }
  }

  void paint_lines()
  {
    for (const auto& line : lines)
    {
      // Paints the lines
      DrawLine(nodes[line.from].x, nodes[line.from].y, nodes[line.to].x, nodes[line.to].y, olc::CYAN);

      // Paints the little triangles to indicate line direction

      // Distance between source and target (also indicates direcion by sign (+/-))
      olc::vf2d direction = nodes[line.from] - nodes[line.to];

      // Calculating the tip of the triangle that touches the node (position + (direction * (radius / length)))
      olc::vi2d one = nodes[line.to] + (direction * (radius / direction.mag()));

      // This is the point further down the line (literally)
      olc::vi2d two = nodes[line.to] + (direction * ((radius + 15) / direction.mag()));

      // Helper position to calculate the positions of the corners off the line
      olc::vf2d helper = nodes[line.to] + (direction * ((radius + 20) / direction.mag()));
      olc::vf2d helperDirection = helper - nodes[line.from];

      // TODO: adjust triangle wing positions to be more visually pleasing
      // These are the positions to the left/right of the line, forming a complete triangle
      // * The cast to int is only there to stop the compiler from complaining about narrowing conversion from float to int
      olc::vi2d three = {int(helper.x - ((helperDirection.y / direction.mag()) * 10)), int(helper.y + ((helperDirection.x / direction.mag()) * 10))};
      olc::vi2d four = {int(helper.x + ((helperDirection.y / direction.mag()) * 10)), int(helper.y - ((helperDirection.x / direction.mag()) * 10))};

      FillTriangle(one, two, three, olc::CYAN);
      FillTriangle(one, two, four, olc::CYAN);

      // Draw(helper, olc::Pixel(rand() % 256, rand() % 256, rand() % 256)); // dbg
      // DrawLine(one, two, olc::Pixel(rand() % 256, rand() % 256, rand() % 256)); // dbg

      // Paints the distance onto the middle of the line
      DrawStringProp((nodes[line.from].x + nodes[line.to].x) / 2 - 8, (nodes[line.from].y + nodes[line.to].y) / 2 - 8, std::to_string(line.length), olc::WHITE, 2);
    }
  }

  void paint_nodes()
  {
    int hovered_node = 0;

    for (const auto& node : nodes)
    {
      // Node color changes if it is the selected node that is being moved around
      FillCircle(node.second.x, node.second.y, radius, (node.first == selected_node ? olc::MAGENTA : olc::Pixel(255, 128, 0)));
      // Draws the number
      DrawStringProp((node.first < 10 ? olc::vi2d{node.second.x - 3, node.second.y - 3} : olc::vi2d{node.second.x - 7, node.second.y - 3}), std::to_string(node.first), olc::BLACK, 1);

      // A node gets an outline on hover execpt in NODE mode
      if (mode != NODE and is_mouse_in_circle(node.second)) hovered_node = node.first;
    }

    if (hovered_node != 0)
    {
      DrawCircle(nodes[hovered_node].x, nodes[hovered_node].y, radius + 4, olc::BLACK);
      DrawCircle(nodes[hovered_node].x, nodes[hovered_node].y, radius + 5, olc::MAGENTA);
      DrawCircle(nodes[hovered_node].x, nodes[hovered_node].y, radius + 6, olc::BLACK);
    }
  }

  void paint_start_and_end()
  {
    // Draws start
    if (start != 0)
    {
      FillRect(nodes[start].x - 38, nodes[start].y - 28, 74, 16, olc::BLACK);
      DrawStringProp(nodes[start].x - 37, nodes[start].y - 27, "Start", olc::GREEN, 2);
    }

    // Draws end
    if (end != 0)
    {
      FillRect(nodes[end].x - 23, nodes[end].y - 28, 44, 16, olc::BLACK);
      DrawStringProp(nodes[end].x - 22, nodes[end].y - 27, "End", olc::GREEN, 2);
    }
  }

  void paint_path()
  {
  }

  // Finds the smallest missing number in this sequence of numbers (node IDs) otherwise a new ID is created
  int generate_node_ID()
  {
    // This is the ID that is expected to be there when we go through all the nodes
    int id = 1;
    for (const auto& node : nodes)
    {
      // We found a node whose ID does not match our expectation (bigger than $id), so we return the wanted ID
      if (node.first != id) return id;
      id++;
    }

    // All nodes have the expected ID, so we generate a new one
    return nodes.size() + 1;
  }

  void increment_line_length()
  {
    if (line_length < 99) line_length++;
  }

  void decrement_line_length()
  {
    if (line_length > 1) line_length--;
  }

  bool do_circles_overlap(const olc::vi2d& circle1, const olc::vi2d& circle2)
  {
    return fabs(pow((circle1.x - circle2.x), 2) + pow((circle1.y - circle2.y), 2)) <= pow(2 * radius, 2);
  }
  bool do_circles_overlap(const int& x1, const int& y1, const int& x2, const int& y2)
  {
    return fabs(pow((x1 - x2), 2) + pow((y1 - y2), 2)) <= pow(2 * radius, 2);
  }

  bool is_mouse_in_circle(const olc::vi2d& circle)
  {
    return fabs(pow((circle.x - GetMouseX()), 2) + pow((circle.y - GetMouseY()), 2)) < pow(radius, 2);
  }
  bool is_mouse_in_circle(const int& x, const int& y)
  {
    return fabs(pow((x - GetMouseX()), 2) + pow((y - GetMouseY()), 2)) < pow(radius, 2);
  }

  bool is_mouse_in_rect(const olc::vi2d& position, const olc::vi2d& dimensions)
  {
    if (GetMouseX() < position.x or GetMouseY() < position.y or GetMouseX() > position.x + dimensions.x or GetMouseY() > position.y + dimensions.y) return false;
    return true;
  }
  bool is_mouse_in_rect(const int& x, const int& y, const int& width, const int& height)
  {
    if (GetMouseX() < x or GetMouseY() < y or GetMouseX() > x + width or GetMouseY() > y + height) return false;
    return true;
  }
};

int main()
{
  PGE_graph_visualiser instance;

  if (instance.Construct(1'280, 820, 1, 1)) instance.Start();

  return 0;
}
