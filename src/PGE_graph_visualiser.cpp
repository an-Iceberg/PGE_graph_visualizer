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
  int UISectionHeight = 100;
  int radius = 12;
  int selectedNode = 0;
  int lineLength = 1;
  int start = 0;
  int end = 0;
  mode mode = MOVE;
  std::vector<line> lines;
  std::map<int, olc::vi2d> nodes; // The key serves as the ID of the node

public:
  bool OnUserCreate() override
  {
    nodes[1] = {150, 150};
    nodes[2] = {200, 600};
    nodes[3] = {1000, 600};
    nodes[4] = {1000, 300};

    lines.push_back(line(1, 3, 7));
    lines.push_back(line(3, 2, 14));

    return true;
  }

  bool OnUserUpdate(float elapsedTime) override
  {
    if (IsFocused())
    {
      Clear(olc::BLACK);

      HandleModeChange();
      HandleInput();

      PaintLines();
      PaintNodes();

      PaintUI();
    }

    return true;
  }


  // ----------


  // TODO: implement input handling
private:
  void HandleModeChange()
  {
    if (GetKey(olc::M).bPressed) mode = MOVE;
    else if (GetKey(olc::N).bPressed) mode = NODE;
    else if (GetKey(olc::L).bPressed) mode = LINE;
    else if (GetKey(olc::P).bPressed) mode = PATH;
  }

  void HandleInput()
  {
    if (mode == MOVE)
    {
      // The node which the mouse is hovering over is being selected
      if (GetMouse(0).bPressed)
      {
        for (const auto& node : nodes) if (IsMouseInCircle(node.second)) selectedNode = node.first;
      }
      // Moving the node around
      else if (GetMouse(0).bHeld and selectedNode != 0) nodes[selectedNode] = {GetMouseX(), GetMouseY()};
      // Releasing the node from our iron grip
      else if (GetMouse(0).bReleased) selectedNode = 0;
    }
    else if (mode == NODE)
    {
      // Create node on primary mouse click
      if (GetMouseY() > UISectionHeight and GetMouse(0).bPressed)
      {
        // Only create a new node if it does not overlap with any existing one
        for (const auto& node : nodes)
        {
          // Giving the mouse a bit of a deadzone around it just to be safe
          if (DoCirclesOverlap(node.second, {GetMouseX() + 2, GetMouseY() + 2})) return;
        }

        // Creating a new node
        nodes[GenerateNodeID()] = {GetMouseX(), GetMouseY()};
      }
      // Deleting a node and all lines coming/going from/to it
      else if (GetMouse(1).bPressed)
      {
        // Finding the node
        for (const auto& node : nodes)
        {
          if (DoCirclesOverlap(node.second, {GetMouseX(), GetMouseY()}))
          {
            // Deleting all lines associated with said node
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
            break;
          }
        }
      }

      // If user presses delete or backspace they delete all nodes and lines
      if (GetKey(olc::BACK).bPressed or GetKey(olc::DEL).bPressed)
      {
        lines.clear();
        nodes.clear();
      }
    }
    else if (mode == LINE)
    {
      if (GetMouse(0).bPressed)
      {
      }
      else if (GetMouse(0).bHeld)
      {
      }
      else if (GetMouse(0).bReleased)
      {
      }

      // If user presses delete or backspace they delete all lines
      if (GetKey(olc::BACK).bPressed or GetKey(olc::DEL).bPressed) lines.clear();
    }
    else if (mode == PATH)
    {
      if (GetMouse(0).bPressed)
      {
      }

      if (GetMouse(1).bPressed)
      {
      }

      if (GetKey(olc::ENTER).bPressed)
      {
      }

      // Clearing the start and end node
      if (GetKey(olc::BACK).bPressed or GetKey(olc::DEL).bPressed)
      {
      }
    }
  }

  void PaintUI()
  {
    // Draws a border around the UI section
    for (int i = 0; i < 6; i++)
    {
      DrawRect(0 + i, 0 + i, ScreenWidth() - ((2 * i) + 1), UISectionHeight - ((2 * i) + 1), olc::Pixel(128, 0, 255));
    }
    FillRect(6, 6, ScreenWidth() - 12, UISectionHeight - 12, olc::VERY_DARK_CYAN);

    // TODO: rework mode UI element
    // Painting the mode selector
    if (mode == MOVE)
    {
      DrawStringProp({10, 10}, "Mode:", olc::GREY, 2);
      DrawString({90, 10}, " M  N  L  P", olc::MAGENTA, 2);
      DrawString({91, 10}, "[ ]", olc::WHITE, 2);
      DrawStringProp({285, 10}, "- Move", olc::GREY, 2);

      if (selectedNode == 0)
      {
        DrawStringProp({10, 29}, "Left Mouse: hold to move a node around", olc::GREY, 2);
        DrawStringProp({10, 29}, "Left Mouse:", olc::MAGENTA, 2);
      }
      else DrawStringProp({10, 29}, "Release to place node", olc::GREY, 2);
    }
    else if (mode == NODE)
    {
      DrawStringProp({10, 10}, "Mode:", olc::GREY, 2);
      DrawString({90, 10}, " M  N  L  P", olc::MAGENTA, 2);
      DrawString({139, 10}, "[ ]", olc::WHITE, 2);
      DrawStringProp({285, 10}, "- Node", olc::GREY, 2);

      DrawStringProp({10, 29}, "Left Mouse: create a node", olc::GREY, 2);
      DrawStringProp({10, 29}, "Left Mouse", olc::MAGENTA, 2);
      DrawStringProp({10, 48}, "Right Mouse: delete a node", olc::GREY, 2);
      DrawStringProp({10, 48}, "Right Mouse", olc::MAGENTA, 2);
    }
    else if (mode == LINE)
    {
      DrawStringProp({10, 10}, "Mode:", olc::GREY, 2);
      DrawString({90, 10}, " M  N  L  P", olc::MAGENTA, 2);
      DrawString({187, 10}, "[ ]", olc::WHITE, 2);
      DrawStringProp({285, 10}, "- Line", olc::GREY, 2);
    }
    else if (mode == PATH)
    {
      DrawStringProp({10, 10}, "Mode:", olc::GREY, 2);
      DrawString({90, 10}, " M  N  L  P", olc::MAGENTA, 2);
      DrawString({235, 10}, "[ ]", olc::WHITE, 2);
      DrawStringProp({285, 10}, "- Path", olc::GREY, 2);
    }

    // TODO: implement hover differently
    // Hover on 'M'
    if (mode != MOVE and IsMouseInRect({103, 7}, {18, 19}))
    {
      DrawString({91, 10}, "[ ]", olc::GREY, 2);
      if (GetMouse(0).bPressed) mode = MOVE;
    }
    // Hover on 'N'
    else if (mode != NODE and IsMouseInRect({151, 7}, {18, 19}))
    {
      DrawString({139, 10}, "[ ]", olc::GREY, 2);
      if (GetMouse(0).bPressed) mode = NODE;
    }
    // Hover on 'L'
    else if (mode != LINE and IsMouseInRect({200, 7}, {18, 19}))
    {
      DrawString({187, 10}, "[ ]", olc::GREY, 2);
      if (GetMouse(0).bPressed) mode = LINE;
    }
    // Hover on 'P'
    else if (mode != PATH and IsMouseInRect({247, 7}, {18, 19}))
    {
      DrawString({235, 10}, "[ ]", olc::GREY, 2);
      if (GetMouse(0).bPressed) mode = PATH;
    }

    // std::string mouseCoordinates = "x:{x} y:{y}";
    // std::unordered_map<std::string, std::string> coordinates {
    //   {"{x}", std::to_string(GetMouseX())},
    //   {"{y}", std::to_string(GetMouseY())},
    // };
    // DrawStringProp(ScreenWidth() - 83, 5, str_replace(mouseCoordinates, coordinates), olc::GREY);
  }

  void PaintLines()
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
      // TODO: try rotating the helper vector instead of this weird stuff
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

  void PaintNodes()
  {
    int nodeHoverID = 0;

    for (const auto& node : nodes)
    {
      // Node color changes if it is the selected node that is being moved around
      FillCircle(node.second.x, node.second.y, radius, (node.first == selectedNode ? olc::MAGENTA : olc::Pixel(255, 128, 0)));
      // TODO: adjust number position for numbers larger than 10 to fit into circle properly
      DrawString(node.second.x - 7, node.second.y - 6, std::to_string(node.first), olc::BLACK, 2);

      // A node gets an outline on hover but ony in MOVE mode
      if (mode == MOVE and IsMouseInCircle(node.second)) nodeHoverID = node.first;
    }

    // Painting circles around hovered node (only if it isn't being moved)
    if (mode == MOVE and selectedNode == 0 and nodeHoverID != 0)
    {
      DrawCircle(nodes[nodeHoverID].x, nodes[nodeHoverID].y, radius + 4, olc::BLACK);
      DrawCircle(nodes[nodeHoverID].x, nodes[nodeHoverID].y, radius + 5, olc::MAGENTA);
      DrawCircle(nodes[nodeHoverID].x, nodes[nodeHoverID].y, radius + 6, olc::BLACK);
    }
  }

  // FIX: sometimes, when all nodes are cleared, only one node with id 1 is being created
  // Finds the smallest missing number in this sequence of numbers (node IDs) otherwise a new ID is created
  int GenerateNodeID()
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

  bool DoCirclesOverlap(const olc::vi2d& circle1, const olc::vi2d& circle2) {
    return fabs(pow((circle1.x - circle2.x), 2) + pow((circle1.y - circle2.y), 2)) <= pow(2 * radius, 2);
  }
  bool DoCirclesOverlap(const int& x1, const int& y1, const int& x2, const int& y2) {
    return fabs(pow((x1 - x2), 2) + pow((y1 - y2), 2)) <= pow(2 * radius, 2);
  }

  bool IsMouseInCircle(const olc::vi2d& circle) {
    return fabs(pow((circle.x - GetMouseX()), 2) + pow((circle.y - GetMouseY()), 2)) < pow(radius, 2);
  }
  bool IsMouseInCircle(const int& x, const int& y) {
    return fabs(pow((x - GetMouseX()), 2) + pow((y - GetMouseY()), 2)) < pow(radius, 2);
  }

  bool IsMouseInRect(const olc::vi2d& position, const olc::vi2d& dimensions) {
    if (GetMouseX() < position.x) return false;
    if (GetMouseY() < position.y) return false;
    if (GetMouseX() > position.x + dimensions.x) return false;
    if (GetMouseY() > position.y + dimensions.y) return false;

    return true;
  }
  bool IsMouseInRect(const int& x, const int& y, const int& width, const int& height) {
    if (GetMouseX() < x) return false;
    if (GetMouseY() < y) return false;
    if (GetMouseX() > x + width) return false;
    if (GetMouseY() > y + height) return false;

    return true;
  }

  // Replaces the given keys with the values of 'replacements' in the 'string'
  // std::string str_replace(std::string string, std::unordered_map<std::string, std::string> replacements)
  // {
  //   for (const auto& replacement : replacements)
  //   {
  //     string.replace(string.find(replacement.first), std::string(replacement.first).size(), replacement.second);
  //   }

  //   return string;
  // }
};

int main()
{
  PGE_graph_visualiser instance;

  if (instance.Construct(1280, 820, 1, 1)) instance.Start();

  return 0;
}
