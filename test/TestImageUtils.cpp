#include "TestImageUtils.h"
#include <util/ImageUtils.h>

void TestImageUtils::generateDitchMatRecTestCase()
{
    //cv::Mat mat = imageUtils::generateSpiralDitchMat(2);
    //std::cout << mat << std::endl;
    cv::Mat mat = imageUtils::generateSpiralDitchMat(3);
    std::cout << mat << std::endl;
    //mat = imageUtils::generateSpiralDitchMat(4);
    //std::cout << mat << std::endl;
    /*mat = imageUtils::generateSpiralDitchMat(5);
    std::cout << mat << std::endl;
    mat = imageUtils::generateSpiralDitchMat(6);
    std::cout << mat << std::endl;
    mat = imageUtils::generateSpiralDitchMat(7);
    std::cout << mat << std::endl;
    mat = imageUtils::generateSpiralDitchMat(8);
    std::cout << mat << std::endl;
    mat = imageUtils::generateSpiralDitchMat(9);
    std::cout << mat << std::endl;
    mat = imageUtils::generateSpiralDitchMat(10);
    std::cout << mat << std::endl;
*/
    /*mat = imageUtils::generateBayerDitchMatRec(1);
    std::cout << mat << std::endl;
    mat = imageUtils::generateBayerDitchMatRec(2);
    std::cout << mat << std::endl;*/
    /*mat = imageUtils::generateBayerDitchMatRec(3);
    std::cout << mat << std::endl;
    mat = imageUtils::generateBayerDitchMatRec(4);
    std::cout << mat << std::endl;*/
}
