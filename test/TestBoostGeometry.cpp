#include <iostream>
#include <deque>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>

#include <boost/foreach.hpp>


int main()
{
    typedef boost::geometry::model::d2::point_xy<double> point;
    typedef boost::geometry::model::polygon<point> polygon;
    typedef boost::geometry::model::box<point> box;
    typedef boost::geometry::model::segment<point> segment;
    typedef boost::geometry::model::linestring<point> line;

    polygon green, blue;
    line line1{ {-5.0, -5.0}, {5.0, 5.0} };

    /*boost::geometry::read_wkt(
        "POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2,4.1 3,5.3 2.6,5.4 1.2,4.9 0.8,2.9 0.7,2 1.3)"
        "(4.0 2.0, 4.2 1.4, 4.8 1.9, 4.4 2.2, 4.0 2.0))", green);*/
    boost::geometry::read_wkt(
        "POLYGON((-10 -10, -10 10, 10 10, 10 -10))", green);

    point p1{ -15, 20 };


    //boost::geometry::read_wkt(
        //"POLYGON((4.0 -0.5 , 3.5 1.0 , 2.0 1.5 , 3.5 2.0 , 4.0 3.5 , 4.5 2.0 , 6.0 1.5 , 4.5 1.0 , 4.0 -0.5))", blue);
    //boost::geometry::read_wkt(
        //"LINESTRING((-5.0 -5.0, 5.0 5.0))", line1);

    std::deque<polygon> output;
    std::vector<point> result;
    boost::geometry::intersection(line1, green, result);

    int i = 0;
    std::cout << "green && line: " << result.size() << std::endl;
    BOOST_FOREACH(point const& p, result)
    {
        std::cout << i++ << " point: " << p.x() << ", " << p.y() << std::endl;
    }
    /*std::cout << "green && blue: " << output.size() << std::endl;
    BOOST_FOREACH(polygon const& p, output)
    {
        std::cout << i++ << " polygon's area: " << boost::geometry::area(p) << std::endl;
    }*/

    boost::multiprecision::cpp_dec_float_50 decimal("0.45");
    double dbl = 0.45; //for comparison

    std::cout << std::fixed << std::setprecision(50) << "boost:   " << decimal << std::endl;
    std::cout << "double:  " << dbl << std::endl;

    return 0;
}