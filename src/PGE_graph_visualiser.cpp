#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <map>

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
  olc::vi2d* selectedNode = nullptr;
  std::vector<line> lines;
  std::map<int, olc::vi2d> nodes; // The map index serves as the ID of the node

public:
  bool OnUserCreate() override
  {
    nodes[1] = {150, 150};
    nodes[2] = {200, 300};
    nodes[3] = {1000, 600};
    nodes[4] = {1000, 300};

    lines.push_back(line(1, 3, 7));
    lines.push_back(line(3, 2, 14));

    return true;
  }

  bool OnUserUpdate(float elapsedTime) override
  {
    Clear(olc::BLACK);

    PaintUI();

    PaintLines();
    PaintNodes();

    return true;
  }


  // ----------


private:
  void PaintUI()
  {
    FillRect(0, 0, ScreenWidth(), UISectionHeight, olc::Pixel(128, 0, 255));

    std::string mouseCoordinates = "x:%x% y:%y%";
    mouseCoordinates.replace(mouseCoordinates.find("%x%"), sizeof("%x%") - 1, std::to_string(GetMouseX()));
    mouseCoordinates.replace(mouseCoordinates.find("%y%"), sizeof("%y%") - 1, std::to_string(GetMouseY()));
    DrawStringProp(ScreenWidth() - 83, 5, mouseCoordinates);
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
      // These are the positions to the left/right of the line, forming a complete triangle
      // * The cast to int is only there to stop the compiler from complaining about narrowing conversion from float to int
      olc::vi2d three = {int(helper.x - (helperDirection.y / direction.mag()) * 10), int(helper.y + (helperDirection.x / direction.mag()) * 10)};
      olc::vi2d four = {int(helper.x + (helperDirection.y / direction.mag()) * 10), int(helper.y - (helperDirection.x / direction.mag()) * 10)};

      FillTriangle(one, two, three, olc::CYAN);
      FillTriangle(one, two, four, olc::CYAN);

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
      DrawString(node.second.x - 7, node.second.y - 6, std::to_string(node.first), olc::BLACK, 2);

      // A node gets an outline on hover
      if (IsMouseInCircle(node.second)) DrawCircle(node.second.x, node.second.y, radius + 5, olc::MAGENTA);
    }
  }

  // Returns true if two nodes overlap
  bool DoCirclesOverlap(const olc::vi2d& circle1, const olc::vi2d& circle2)
  {
    return fabs(pow((circle1.x - circle2.x), 2) + pow((circle1.y - circle2.y), 2)) <= pow(4 * radius, 2);
  }

  // Returns true if the mouse is inside a specified node
  bool IsMouseInCircle(const olc::vi2d& circle)
  {
    return fabs(pow((circle.x - GetMouseX()), 2) + pow((circle.y - GetMouseY()), 2)) < pow(radius, 2);
  }
};

int main()
{
  PGE_graph_visualiser instance;

  if (instance.Construct(1280, 820, 1, 1)) instance.Start();

  return 0;
}
