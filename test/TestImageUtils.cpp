#include "TestImageUtils.h"
#include <util/ImageUtils.h>

void TestImageUtils::generateDitchMatRecTestCase()
{
    //cv::Mat mat = imageUtils::generateSpiralDitchMat(2);
    //std::cout << mat << std::endl;
    //int grades;
    //cv::Mat mat = imageUtils::generateSpiralDitchMat(20, grades);
    //std::cout << mat << std::endl;
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

    //cv::Mat mat = imageUtils::generateCircleMat(20);
    //std::cout << mat << std::endl;
}

void TestImageUtils::generateRoundSpiralMatTestCase()
{
    /*cv::Mat mat = imageUtils::generateRoundSpiralMat(5);
    std::cout << "mat:" << std::endl;
    std::cout << mat << std::endl;
    cv::Mat rotMat = imageUtils::rotateMat(mat, 45);
    std::cout << "rotMat:" << std::endl;
    std::cout << rotMat << std::endl;*/
    //imageUtils::generateRoundSpiralMat(6);
}

void TestImageUtils::generateRotatedPatternTestCase()
{
    cv::Mat mat = imageUtils::generateRoundSpiralMat(4);
    std::cout << "mat:" << std::endl;
    std::cout << mat << std::endl;
    imageUtils::generateRotatedPattern45(mat);
    //cv::Mat rotMat = imageUtils::generateRotatedPattern2(mat, 4, 45);
    //std::cout << "rotMat:" << std::endl;
    //std::cout << rotMat << std::endl;
}
