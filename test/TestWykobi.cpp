#include <iostream>
#include <deque>

#include <wykobi.hpp>
#include <wykobi_gui.hpp>
#include <wykobi_graphics_opengl.hpp>

int main(int argc, char** argv)
{
    wykobi::polygon<float, 2> polygon(4);
    polygon[0] = {-2, -3};
    polygon[1] = {-3, 2};
    polygon[2] = {2, 3};
    polygon[3] = {4, -2};

    wykobi::point2d<float> point;
    point.x = -1.0;
    point.y = 5.0;

    wykobi::point2d<float> origin;

    wykobi::point2d<float> intersectpoint = wykobi::closest_point_on_polygon_from_point(polygon, point);
    std::cout << intersectpoint.x << ", " << intersectpoint.y << std::endl;
    
    intersectpoint = wykobi::closest_point_on_polygon_from_point(polygon, point);
    std::cout << intersectpoint.x << ", " << intersectpoint.y << std::endl;

    //wykobi::wykobi_graphics_opengl<float> graphic(800, 600, wykobi::DrawingMode::eSolid);
    //graphic.draw(polygon);
    //wykobi_window window(argc, argv);

    return 0;
}