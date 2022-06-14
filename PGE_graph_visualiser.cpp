// TODO: better comments/documentation of functions and everything
// TODO: use vi2d whenever possible
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <set>
#include <stack>
#include <vector>
#include <climits>
#include <queue>
#include <unordered_map>

enum Mode
{
  MOVE,
  VERTEX,
  EDGE,
  DIJKSTRA
};

struct s_Vertex
{
  float positionX;
  float positionY;
  int id;

  // Constructor
  s_Vertex(float x, float y, int _id)
  {
    this->positionX = x;
    this->positionY = y;
    this->id = _id;
  }
};

struct s_Edge
{
  // Id of the source vertex
  int source;

  // Id of the target vertex
  int target;

  int length;

  // Constructor
  s_Edge(int _source, int _target, int _length)
  {
    this->source = _source;
    this->target = _target;
    this->length = _length;
  }
};

struct s_Point
{
  int id;

  // The id of the parent
  int parent;

  int distanceToParent;
  bool visited;

  // A list of all the children this point has
  std::vector<s_Point*> children;

  s_Point(int _id, int _parent, int distance_to_parent)
  {
    this->id = _id;
    this->parent = _parent;
    this->distanceToParent = distance_to_parent;
    this->visited = false;
  }
};

class GraphingTool : public olc::PixelGameEngine
{
public:
  GraphingTool()
  {
    sAppName = "Graphing Tool";
  }

private:
  // Contains the data for the vertices
  std::vector<s_Vertex> v_Vertices;

  // Contains the data for the edges
  std::vector<s_Edge> v_Edges;

  std::set<int> s_Indices;

  // Contains the ids of the vertices that form the shortest path from i_Start to i_End
  std::vector<int> v_Path;

  s_Vertex *p_SelectedVertex = nullptr;

  int i_Radius = 15;
  int i_SelectedVertex = -1;
  int i_EdgeLength = 1;
  Mode e_Mode = MOVE;

  // Id of the selected starting vertex
  int i_Start = -1;

  // Id of the selected ending vertex
  int i_End = -1;

  olc::vf2d mouse;

  bool b_ChangeHasOccurred = false;

public:
  bool OnUserCreate() override
  {
    // Debug data, remove when deploying to production
    v_Vertices = {
      s_Vertex(258, 467, 0),
      s_Vertex(474, 627, 1),
      s_Vertex(53, 663, 2),
      s_Vertex(461, 353, 3),
      s_Vertex(325, 146, 4),
      s_Vertex(232, 311, 5),
      s_Vertex(53, 461, 6),
      s_Vertex(568, 488, 7),
      s_Vertex(744, 441, 8),
      s_Vertex(564, 241, 9),
      s_Vertex(454, 63, 10),
      s_Vertex(830, 91, 11),
      s_Vertex(880, 317, 12),
      s_Vertex(1211, 59, 13),
      s_Vertex(1176, 462, 14),
      s_Vertex(855, 608, 15),
      s_Vertex(1216, 182, 16)
    };

    v_Edges = {
      s_Edge(13, 16, 1),
      s_Edge(16, 14, 1),
      s_Edge(14, 15, 1),
      s_Edge(12, 11, 1),
      s_Edge(11, 10, 1),
      s_Edge(9, 8, 1),
      s_Edge(8, 7, 1),
      s_Edge(7, 3, 1),
      s_Edge(4, 5, 1),
      s_Edge(5, 0, 1),
      s_Edge(0, 1, 1),
      s_Edge(6, 2, 1),
      s_Edge(1, 6, 2),
      s_Edge(10, 9, 2),
      s_Edge(3, 4, 2),
      s_Edge(15, 12, 2),
      s_Edge(14, 12, 4),
      s_Edge(12, 10, 4),
      s_Edge(9, 3, 4),
      s_Edge(3, 5, 4),
      s_Edge(0, 6, 4),
      s_Edge(1, 2, 4),
      s_Edge(16, 12, 5),
      s_Edge(8, 3, 5),
      s_Edge(14, 8, 5),
      s_Edge(5, 6, 6),
      s_Edge(16, 11, 7),
      s_Edge(12, 8, 7),
      s_Edge(10, 4, 9),
      s_Edge(7, 1, 8),
      s_Edge(15, 1, 10),
      s_Edge(15, 8, 9),
      s_Edge(3, 0, 7),
      s_Edge(7, 0, 8),
      s_Edge(13, 11, 7)
    };

    s_Indices = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

    i_Start = 12;
    i_End = 2;
    e_Mode = DIJKSTRA;

    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override
  {
    // Getting and storing mouse input for one frame
    mouse.x = GetMouseX();
    mouse.y = GetMouseY();

    UserInput();
    VertexCollision();
    DrawingRoutine();
    return true;
  }

  // Handles user input
  void UserInput()
  {
    // If any substantial changes have occured to the graph, the shortesst path may no longer be valid
    // This flag is for detecting such changes and removing the shortest path data in that case
    b_ChangeHasOccurred = false;

    // Changing modes via arrow keys
    if (GetKey(olc::RIGHT).bPressed)
    {
      switch (e_Mode)
      {
        case MOVE:
          e_Mode = VERTEX;
        break;

        case VERTEX:
          e_Mode = EDGE;
        break;

        case EDGE:
          e_Mode = DIJKSTRA;
        break;
      }
    }

    if (GetKey(olc::LEFT).bPressed)
    {
      switch (e_Mode)
      {
        case DIJKSTRA:
          e_Mode = EDGE;
        break;

        case EDGE:
          e_Mode = VERTEX;
        break;

        case VERTEX:
          e_Mode = MOVE;
        break;
      }
    }

    // Different modes handle user input differently
    switch (e_Mode)
    {
      // Allows the user to move vertices around via mouse input
      case MOVE:
        // Select vertex by pressing left mouse
        if (GetMouse(0).bPressed)
        {
          SelectVertex();
        }

        // Move vertex around by holding left mouse
        if (GetMouse(0).bHeld)
        {
          MoveVertex();
        }

        if (GetMouse(0).bReleased)
        {
          p_SelectedVertex = nullptr;
        }
      break;

      // Allows user to create/delete vertices
      case VERTEX:
        // The maximum number of vertices allowed is 99
        // Create vertex with left mouse
        if (v_Vertices.size() < 100 && GetMouse(0).bPressed)
        {
          CreateNewVertex();
        }

        // Delete vertex with right mouse
        if (GetMouse(1).bPressed)
        {
          DeleteVertex();
        }
      break;

      // Allows user to create/delete edges
      case EDGE:
        // Create new edge with left mouse
        if (GetMouse(0).bPressed)
        {
          CreateNewEdge();
        }

        // Delete edge with right mouse
        if (GetMouse(1).bPressed)
        {
          DeleteEdge();
        }
      break;

      // Allows user to set a start and end point and let the program find the shortest path between the two
      case DIJKSTRA:
        // Left mouse button sets start vertex
        if (GetMouse(0).bPressed)
        {
          SetStart();
        }

        // Right mouse button sets end vertex
        if (GetMouse(1).bPressed)
        {
          SetEnd();
        }

        // Enter key finds the shortest path between start and end vertex
        if (GetKey(olc::ENTER).bReleased)
        {
          FindShortestPath();
        }
      break;
    }

    // Pressing A key increases edge length
    if (i_EdgeLength > 1 && GetKey(olc::A).bPressed)
    {
      i_EdgeLength--;
    }

    // Pressing D key reduces edge length
    if (i_EdgeLength < 20 && GetKey(olc::D).bPressed)
    {
      i_EdgeLength++;
    }

    // Pressing UP key increases vertex radius
    if (i_Radius < 20 && GetKey(olc::UP).bPressed)
    {
      i_Radius++;
    }

    // Pressing DOWN key reduces vertex radius
    if (i_Radius > 12 && GetKey(olc::DOWN).bPressed)
    {
      i_Radius--;
    }

    // User can clear all graph data by pressing backspace
    if (!v_Vertices.empty() && GetKey(olc::BACK).bPressed)
    {
      v_Vertices.clear();
      s_Indices.clear();
      v_Edges.clear();
      v_Path.clear();
      i_Start = -1;
      i_End = -1;
      return;
    }

    // If any substantial change has occured to the graph, the shortest path data is cleared
    if (!v_Path.empty() && b_ChangeHasOccurred)
    {
      v_Path.clear();
    }
  }

  /**
   * @brief Prevents two vertices from having the same position by pushing them away from each other using realistic physics
   */
  void VertexCollision()
  {
    for (auto &vertex : v_Vertices)
    {
      for (auto &target : v_Vertices)
      {
        if (vertex.id != target.id)
        {
          if (DoCirclesOverlap(vertex.positionX, vertex.positionY, i_Radius, target.positionX, target.positionY, i_Radius))
          {
            float fDistance = sqrtf((vertex.positionX - target.positionX) * (vertex.positionX - target.positionX) + (vertex.positionY - target.positionY) * (vertex.positionY - target.positionY));
            float fOverlap = 0.5f * (fDistance - (i_Radius * 2));

            vertex.positionX -= fOverlap * (vertex.positionX - target.positionX) / fDistance;
            vertex.positionY -= fOverlap * (vertex.positionY - target.positionY) / fDistance;

            target.positionX += fOverlap * (vertex.positionX - target.positionX) / fDistance;
            target.positionY += fOverlap * (vertex.positionY - target.positionY) / fDistance;
          }
        }
      }
    }
  }

  // Draws the graph to the screen
  void DrawingRoutine()
  {
    // Clears the screen
    Clear(olc::DARK_BLUE);

    std::string mode;

    // Selects the text to be drawn based upon which mode is active
    switch (e_Mode)
    {
      case MOVE: mode = "  move >"; break;
      case VERTEX: mode = "< vertex >"; break;
      case EDGE: mode = "< edge >"; break;
      case DIJKSTRA: mode = "< find shortest path from Start to End"; break;
    }

    // Drawing the edges with length and direction
    for (auto const &edge : v_Edges)
    {
      // The coorditates of the source edge
      float sourceX = GetX(edge.source);
      float sourceY = GetY(edge.source);

      // The coordinates of the target edge
      float targetX = GetX(edge.target);
      float targetY = GetY(edge.target);

      // Distance between source and targen in x direction (also indicates direction by the sign (+/-))
      float directionX = sourceX - targetX;

      // Distance between source and targen in y direction (also indicates direction by the sign (+/-))
      float directionY = sourceY - targetY;

      // Distance between source and target (by using pythagoras' theorem)
      float length = sqrtf((directionX) * (directionX) + (directionY) * (directionY));

      // Calculating the positions of the triangle corners
      // positionTriangleCorner = position + (direction * (radius / length))
      // x1/y1 are touching the circle
      float x1 = targetX + (directionX * (i_Radius / length));
      float y1 = targetY + (directionY * (i_Radius / length));

      // x2/y2 are on the edge
      float x2 = targetX + (directionX * ((i_Radius + 15.0f) / length));
      float y2 = targetY + (directionY * ((i_Radius + 15.0f) / length));

      // Helper positions to calculate position of floading triangel corners
      float xh = targetX + (directionX * ((i_Radius + 20.0f) / length));
      float yh = targetY + (directionY * ((i_Radius + 20.0f) / length));

      float directionHX = xh - sourceX;
      float directionHY = yh - sourceY;

      // positionTriangleCornerFloating = phelper position +- (direction / length), aka tangent
      // These positions are floating to the left/right of the edge
      float x3 = xh - (directionHY / length) * 10.0f;
      float y3 = yh + (directionHX / length) * 10.0f;
      float x4 = xh + (directionHY / length) * 10.0f;
      float y4 = yh - (directionHX / length) * 10.0f;

      // TODO: refactor these drawing routines to be more concise and efficient; we are using too many if statements
      // Drawing the edges (color depends on mode and whether it's in the path)
      if (e_Mode == DIJKSTRA && EdgeIsInPath(edge.source, edge.target))
      {
        DrawLine(sourceX, sourceY, targetX, targetY, olc::GREEN);
      }
      else
      {
        DrawLine(sourceX, sourceY, targetX, targetY, olc::MAGENTA);
      }

      FillTriangle(x1, y1, x2, y2, x3, y3, olc::MAGENTA);
      FillTriangle(x1, y1, x2, y2, x4, y4, olc::MAGENTA);

      // Draws the length of the edge in the centre of the edge
      DrawString((sourceX + targetX) / 2.0f - 8.0f, (sourceY + targetY) / 2.0f - 8.0f, std::to_string(edge.length), olc::CYAN, 2);
    }

    // Drawing the vertices
    for (auto const &vertex : v_Vertices)
    {
      // Drawing the vertices (color depends on mode and whether it's in the path)
      if (e_Mode == DIJKSTRA && VertexIsInPath(vertex.id))
      {
        FillCircle(vertex.positionX, vertex.positionY, i_Radius, olc::GREEN);
      }
      else
      {
        FillCircle(vertex.positionX, vertex.positionY, i_Radius, olc::Pixel(255, 128, 0));
      }

      // The selected circle is highlighted magenta
      if (vertex.id == i_SelectedVertex)
      {
        FillCircle(vertex.positionX, vertex.positionY, i_Radius, olc::MAGENTA);
      }

      // Adjusting text position so it always looks centerd in the vertex
      if (vertex.id > 9)
      {
        DrawString(vertex.positionX - 15.0f, vertex.positionY - 7.0f, std::to_string(vertex.id), olc::BLACK, 2);
      }
      else
      {
        DrawString(vertex.positionX - 7.0f, vertex.positionY - 7.0f, std::to_string(vertex.id), olc::BLACK, 2);
      }
    }

    // Drawing the 'Start' and 'End' labels if mode is dijkstra
    if (e_Mode == DIJKSTRA)
    {
      for (auto const &vertex : v_Vertices)
      {
        if (vertex.id == i_Start)
        {
          DrawString(vertex.positionX - 40, vertex.positionY - 32.0f, "Start", olc::CYAN, 2);
        }

        if (vertex.id == i_End)
        {
          DrawString(vertex.positionX - 24, vertex.positionY - 32.0f, "End", olc::CYAN, 2);
        }
      }
    }

    // TODO: Draw radius size
    // Drawing mode and edge length information in the top left corner
    DrawString(5.0f, 5.0f,  "Edge length: " + std::to_string(i_EdgeLength), olc::MAGENTA, 2);
    DrawString(5.0f, 25.0f, "Mode: " + mode, olc::MAGENTA, 2);
  }

  // Returns true, if the edge is part of the shortest path
  bool EdgeIsInPath(int const &sourceId, int const &targetId)
  {
    if (v_Path.empty())
    {
      return false;
    }

    for (int i = 0; i < v_Path.size() - 1; i++)
    {
      // If the current path element is the source and the next the target, return true
      if (sourceId == v_Path[i] && targetId == v_Path[i + 1])
      {
        return true;
      }
    }

    return false;
  }

  // Returns true, if a vertex is part of the shortest path
  bool VertexIsInPath(int const &id)
  {
    for (auto const &point : v_Path)
    {
      if (point == id)
      {
        return true;
      }
    }

    return false;
  }

  // The user left-clicks on a vertex, setting it as the starting point of dijkstra's shortest path
  void SetStart()
  {
    for (auto const &vertex : v_Vertices)
    {
      if (IsPointInCircle(vertex.positionX, vertex.positionY, i_Radius, mouse.x, mouse.y))
      {
        i_Start = vertex.id;
        b_ChangeHasOccurred = true;
        return;
      }
    }

    b_ChangeHasOccurred = false;
  }

  // The user right-clicks on a vertex, setting it as the ending point of dijkstra's shortest path
  void SetEnd()
  {
    for (auto const &vertex : v_Vertices)
    {
      if (IsPointInCircle(vertex.positionX, vertex.positionY, i_Radius, mouse.x, mouse.y))
      {
        i_End = vertex.id;
        b_ChangeHasOccurred = true;
        return;
      }
    }

    b_ChangeHasOccurred = false;
  }

  // TODO: extensive testing for the path finding
  // TODO: if a shorter path has been found, overwrite the old shortest path
  // TODO: add a variable "length" so that paths longer than the initial one don't need to be explored and there doesn't need to be a huge list of paths in the "paths" variable
  // Puts the shortest path between the selected start and end point into v_Path
  void FindShortestPath()
  {
    // If either the beginning or end are not set, return
    if (i_Start == -1 || i_End == -1)
    {
      return;
    }

    // Construction data for finding the shortest path from i_Start to i_End
    std::vector<int> path;
    int length = 0;
    std::unordered_map<int, std::vector<int>> paths;

    RecursiveSearchForShortestPath(i_Start, path, length, paths);

    // Finding the shortest among all the paths
    length = INT_MAX;

    for(auto const &path : paths)
    {
      if (path.first < length)
      {
        length = path.first;
      }
    }

    v_Path = paths[length];
  }

  // A recursive function to find all reachable children using DFS (depth first search)
  void RecursiveSearchForShortestPath(int vertex, std::vector<int> path, int length, std::unordered_map<int, std::vector<int>> &paths)
  {
    // Search v_Edges for certain edges that are important
    for (auto &edge : v_Edges)
    {
      // A valid path from i_Start to i_End has been found
      if (edge.source == vertex && edge.target == i_End)
      {
        // Compute the final lenght of the edge
        length += edge.length;

        // Pushing the last vertices onto the path
        path.push_back(edge.source);
        path.push_back(edge.target);

        // Adding the path with length to the set of valid paths
        paths[length] = path;

        // Break recursion
        return;
      }
      // An edge has been found
      else if (edge.source == vertex)
      {
        // Push the vertex onto the path
        path.push_back(edge.source);

        // Increase the length of the path
        length += edge.length;

        // Continue searching for children recursively
        RecursiveSearchForShortestPath(edge.target, path, length, paths);
      }
    }
  }

  // TODO: account for not finding vertex id
  // Returns the position of a vertex on the x axis
  float GetX(int const &id)
  {
    for (auto const &vertex : v_Vertices)
    {
      if (vertex.id == id)
      {
        return vertex.positionX;
      }
    }

    // ! This is not a good solution
    return 0.0f;
  }

  // TODO: account for not finding vertex id
  // Returns the position of a vertex on the y axis
  float GetY(int const &id)
  {
    for (auto const &vertex : v_Vertices)
    {
      if (vertex.id == id)
      {
        return vertex.positionY;
      }
    }

    // ! This is not a good solution
    return 0.0f;
  }

  // The vertex clicked on by the user is marked as selected
  void SelectVertex()
  {
    p_SelectedVertex = nullptr;

    for (auto &vertex : v_Vertices)
    {
      if (IsPointInCircle(vertex.positionX, vertex.positionY, i_Radius, mouse.x, mouse.y))
      {
        p_SelectedVertex = &vertex;
        break;
      }
    }
  }

  // The selected vertex follows the position of the mouse cursor
  void MoveVertex()
  {
    if (p_SelectedVertex != nullptr)
    {
      p_SelectedVertex->positionX = GetMouseX();
      p_SelectedVertex->positionY = GetMouseY();
    }
  }

  // TODO: make this work without s_Indices
  // TODO: refactor this mess
  // Creates a new vertex
  void CreateNewVertex()
  {
    p_SelectedVertex = nullptr;
    bool b_GoodMousePosition = false;

    // If v_Vertices was empty
    if (v_Vertices.size() == 0)
    {
      v_Vertices.push_back(s_Vertex(mouse.x, mouse.y, 0));
      s_Indices.insert(0);
      return;
    }

    // Checks if the mouse is inside a vertex
    for (auto &vertex : v_Vertices)
    {
      if (!IsPointInCircle(vertex.positionX, vertex.positionY, i_Radius, mouse.x, mouse.y))
      {
        b_GoodMousePosition = true;
      }
      else
      {
        b_GoodMousePosition = false;
        break;
      }
    }

    // Only creates new vertex if mouse isn't close or overlapping another one
    if (b_GoodMousePosition)
    {
      int id = 0;

      // Creates appropriate id
      while (s_Indices.count(id) > 0)
      {
        id++;
      }

      v_Vertices.push_back(s_Vertex(mouse.x, mouse.y, id));
      s_Indices.insert(id);
    }

    b_GoodMousePosition = false;
    b_ChangeHasOccurred = true;
  }

  // TODO: make this work without s_Indices
  // Deletes a vertex
  void DeleteVertex()
  {
    p_SelectedVertex = nullptr;
    int _id = -1;

    // Deletes the selected vertex
    for (int i = 0; i < v_Vertices.size(); i++)
    {
      if (IsPointInCircle(v_Vertices[i].positionX, v_Vertices[i].positionY, i_Radius, mouse.x, mouse.y))
      {
        _id = v_Vertices[i].id;
        //s_Indices.erase(v_Vertices[i].id); // This doesn't work for some reason
        v_Vertices.erase(v_Vertices.begin() + i);
        s_Indices.erase(_id);
      }
    }

    // Deletes all edges coming from or going to the selected/deleted vertex
    for (int i = 0; i < v_Edges.size(); i++)
    {
      if (v_Edges[i].source == _id || v_Edges[i].target == _id)
      {
        v_Edges.erase(v_Edges.begin() + i);
        i--;
      }
    }

    b_ChangeHasOccurred = true;
  }

  // TODO: refactor this mess
  // Creates a new edge
  void CreateNewEdge()
  {
    // If no vertex has been selected yet
    if (i_SelectedVertex == -1)
    {

      for (auto &vertex : v_Vertices)
      {
        if (IsPointInCircle(vertex.positionX, vertex.positionY, i_Radius, mouse.x, mouse.y))
        {
          i_SelectedVertex = vertex.id;
        }
      }
    }
    else
    {
      for (auto &vertex : v_Vertices)
      {
        // Don't create an edge from the vertex to itself
        if (vertex.id != i_SelectedVertex)
          if (IsPointInCircle(vertex.positionX, vertex.positionY, i_Radius, mouse.x, mouse.y))
          {
            for (auto const &edge : v_Edges)
            {
              // Don't create a duplicate edge
              if (edge.source == i_SelectedVertex && edge.target == vertex.id)
              {
                i_SelectedVertex = -1;
                return;
              }
              // Don't create a bidirectional edge
              if (edge.target == i_SelectedVertex && edge.source == vertex.id)
              {
                i_SelectedVertex = -1;
                return;
              }
            }

            v_Edges.push_back(s_Edge(i_SelectedVertex, vertex.id, i_EdgeLength));
          }
      }

      i_SelectedVertex = -1;
    }

    b_ChangeHasOccurred = true;
  }

  // Deletes an edge
  void DeleteEdge()
  {
    for (auto const &vertex : v_Vertices)
    {
      if (IsPointInCircle(vertex.positionX, vertex.positionY, i_Radius, mouse.x, mouse.y))
      {
        for (int i = 0; i < v_Edges.size(); i++)
        {
          if (v_Edges[i].source == i_SelectedVertex && v_Edges[i].target == vertex.id)
          {
            v_Edges.erase(v_Edges.begin() + i);
          }
        }
      }
    }

    i_SelectedVertex = -1;
    b_ChangeHasOccurred = true;
  }

  // Returns whether two given circles overlap
  bool DoCirclesOverlap(float const &x1, float const &y1, float const &r1, float const &x2, float const &y2, float const &r2)
  {
    return fabs((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)) <= (r1 + r2) * (r1 + r2);
  }

  // Returns whether a given point is within a circle
  bool IsPointInCircle(float const &circleX, float const &circleY, float const &radius, float const &pointX, float const &pointY)
  {
    return fabs((circleX - pointX) * (circleX - pointX) + (circleY - pointY) * (circleY - pointY)) < (radius * radius);
  }
};

int main()
{
  GraphingTool demo;

  if (demo.Construct(1280, 720, 1, 1))
  {
    demo.Start();
  }

  return 0;
}
