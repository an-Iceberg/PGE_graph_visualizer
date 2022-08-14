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
  PGE_graph_visualiser()
  {
    sAppName = "PGE Graph Visualiser";
  }

private:
  int UISectionHeight = 100;
  int radius = 12;
  mode mode = MOVE;
  olc::vi2d* selectedNode = nullptr;
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
    Clear(olc::BLACK);

    ChangeModeWithKeys();
    HandleClearEverything();
    HandleMouseInput();

    PaintLines();
    PaintNodes();

    PaintUI();

    return true;
  }


  // ----------


  // TODO: implement input handling
private:
  void ChangeModeWithKeys()
  {
    if (GetKey(olc::RIGHT).bPressed) IncrementMode();
    else if (GetKey(olc::LEFT).bPressed) DecrementMode();
    else if (GetKey(olc::M).bPressed) mode = MOVE;
    else if (GetKey(olc::N).bPressed) mode = NODE;
    else if (GetKey(olc::L).bPressed) mode = LINE;
    else if (GetKey(olc::P).bPressed) mode = PATH;
  }

  void HandleClearEverything()
  {
    // If user presses delete or backspace they delete all nodes and lines
    if (GetKey(olc::BACK).bPressed or GetKey(olc::DEL).bPressed)
    {
      lines.clear();
      nodes.clear();
    }
  }

  void HandleMouseInput()
  {
    if (mode == MOVE)
    {
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

        // TODO: generate index properly
        nodes[nodes.size() + 1] = {GetMouseX(), GetMouseY()};
      }
      // Deleting a node and all lines coming/going from/to it
      else if (GetMouse(1).bPressed)
      {
        int nodeIndex = -1;

        // Deleting the node
        for (const auto& node : nodes)
        {
          if (DoCirclesOverlap(node.second, {GetMouseX(), GetMouseY()}))
          {
            nodeIndex = node.first;
            nodes.erase(node.first);
            break;
          }
        }

        // Deleting all lines associated with said node
        for (std::vector<line>::iterator line = lines.begin(); line < lines.end(); line++)
        {
          if (line->from == nodeIndex or line->to == nodeIndex)
          {
            lines.erase(line);
            line--;
          }
        }
      }
    }
    else if (mode == LINE)
    {
    }
    else if (mode == PATH)
    {
    }
  }

  void PaintUI()
  {
    // Draws a border around the UI section
    for (int i = 0; i < 5; i++)
    {
      DrawRect(0 + i, 0 + i, ScreenWidth() - ((2 * i) + 1), UISectionHeight - ((2 * i) + 1), olc::Pixel(128, 0, 255));
    }
    FillRect(5, 5, ScreenWidth() - 10, UISectionHeight - 10, olc::VERY_DARK_CYAN);

    std::string modeString;

    // TODO: implement mouse hover
    // Painting the mode selector
    if (mode == MOVE)
    {
      modeString = "Mode: [M] N  L  P";
      DrawString({10, 10}, modeString, olc::GREY, 2);
      DrawString({106, 10}, "[M]", olc::WHITE, 2);
      DrawString({298, 10}, ">", olc::MAGENTA, 2);
    }
    else if (mode == NODE)
    {
      modeString = "Mode:  M [N] L  P";
      DrawString({10, 10}, modeString, olc::GREY, 2);
      DrawString({154, 10}, "[N]", olc::WHITE, 2);
      DrawString({90, 10}, "<", olc::MAGENTA, 2);
      DrawString({298, 10}, ">", olc::MAGENTA, 2);
    }
    else if (mode == LINE)
    {
      modeString = "Mode:  M  N [L] P";
      DrawString({10, 10}, modeString, olc::GREY, 2);
      DrawString({202, 10}, "[L]", olc::WHITE, 2);
      DrawString({90, 10}, "<", olc::MAGENTA, 2);
      DrawString({298, 10}, ">", olc::MAGENTA, 2);
    }
    else if (mode == PATH)
    {
      modeString = "Mode:  M  N  L [P]";
      DrawString({10, 10}, modeString, olc::GREY, 2);
      DrawString({250, 10}, "[P]", olc::WHITE, 2);
      DrawString({90, 10}, "<", olc::MAGENTA, 2);
    }

    // Hover on 'M'
    if (mode != MOVE and IsMouseInRect({119, 7}, {18, 19}))
    {
      DrawString({122, 10}, "M", olc::MAGENTA, 2);
      if (GetMouse(0).bPressed) mode = MOVE;
    }

    // Hover on 'N'
    if (mode != NODE and IsMouseInRect({168, 7}, {18, 19}))
    {
      DrawString({170, 10}, "N", olc::MAGENTA, 2);
      if (GetMouse(0).bPressed) mode = NODE;
    }

    // Hover on 'L'
    if (mode != LINE and IsMouseInRect({216, 7}, {18, 19}))
    {
      DrawString({218, 10}, "L", olc::MAGENTA, 2);
      if (GetMouse(0).bPressed) mode = LINE;
    }

    // Hover on 'P'
    if (mode != PATH and IsMouseInRect({263, 7}, {18, 19}))
    {
      DrawString({266, 10}, "P", olc::MAGENTA, 2);
      if (GetMouse(0).bPressed) mode = PATH;
    }

    std::string mouseCoordinates = "x:{x} y:{y}";
    std::unordered_map<std::string, std::string> coordinates {
      {"{x}", std::to_string(GetMouseX())},
      {"{y}", std::to_string(GetMouseY())},
    };
    DrawStringProp(ScreenWidth() - 83, 5, str_replace(mouseCoordinates, coordinates), olc::GREY);
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
    for (const auto& node : nodes)
    {
      // Node color changes if it is the selected node that is being moved around
      FillCircle(node.second.x, node.second.y, radius, (&node.second == selectedNode ? olc::MAGENTA : olc::Pixel(255, 128, 0)));
      // TODO: adjust number position for numbers larger than 10 to fit into circle properly
      DrawString(node.second.x - 7, node.second.y - 6, std::to_string(node.first), olc::BLACK, 2);

      // A node gets an outline on hover but ony in MOVE mode
      if (mode == MOVE and IsMouseInCircle(node.second)) DrawCircle(node.second.x, node.second.y, radius + 5, olc::MAGENTA);
    }
  }

  void IncrementMode() {
    if (mode == MOVE) mode = NODE;
    else if (mode == NODE) mode = LINE;
    else if (mode == LINE) mode = PATH;
  }

  void DecrementMode() {
    if (mode == PATH) mode = LINE;
    else if (mode == LINE) mode = NODE;
    else if (mode == NODE) mode = MOVE;
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
  std::string str_replace(std::string string, std::unordered_map<std::string, std::string> replacements)
  {
    for (const auto& replacement : replacements)
    {
      string.replace(string.find(replacement.first), std::string(replacement.first).size(), replacement.second);
    }

    return string;
  }
};

int main()
{
  PGE_graph_visualiser instance;

  if (instance.Construct(1280, 820, 1, 1)) instance.Start();

  return 0;
}
