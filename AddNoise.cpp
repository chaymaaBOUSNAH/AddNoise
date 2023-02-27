// Copyright (C) 2021 Ikomia SAS
// Contact: https://www.ikomia.com
//
// This file is part of the IkomiaStudio software.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "AddNoise.h"

//------------------------------
//----- C_PluginName_Param
//------------------------------
CAddNoise_Param::CAddNoise_Param() : CWorkflowTaskParam()
{
}

void CAddNoise_Param::setParamMap(const UMapString &paramMap)
{
    CWorkflowTaskParam::setParamMap(paramMap);
    // Set parameters values from Ikomia application
    // Each parameter is mapped to a string for identification
    m_noiseType = paramMap.at("m_noiseType");
    sigma = std::stoi(paramMap.at("sigma"));
    mean = std::stoi(paramMap.at("mean"));
    m_salt_p = std::stoi(paramMap.at("m_salt_p"));
    m_pepper_p = std::stoi(paramMap.at("m_pepper_p"));
}

UMapString CAddNoise_Param::getParamMap() const
{
    // Send parameters values to Ikomia application
    auto paramMap = CWorkflowTaskParam::getParamMap();
    paramMap.insert(std::make_pair("sigma", std::to_string(sigma)));
    paramMap.insert(std::make_pair("mean", std::to_string(mean)));
    paramMap.insert(std::make_pair("m_salt_p", std::to_string(m_salt_p)));
    paramMap.insert(std::make_pair("m_pepper_p", std::to_string(m_pepper_p)));
    return paramMap;
}

//------------------------------
//----- CAddNoise
//------------------------------
CAddNoise::CAddNoise() : C2dImageTask()
{
    // Add input/output of the process here
    // Example :
    addOutput(std::make_shared<CImageIO>());
    m_pParam = std::make_shared<CAddNoise_Param>();
}

CAddNoise::CAddNoise(const std::string &name, const std::shared_ptr<CAddNoise_Param> &pParam) : C2dImageTask(name)
{
    m_pParam = std::make_shared<CAddNoise_Param>(*pParam);
}

size_t CAddNoise::getProgressSteps()
{
    return 1;
}

void CAddNoise::run()
{
    beginTaskRun();
    auto pInput = std::dynamic_pointer_cast<CImageIO>(getInput(0));
    auto pParam = std::dynamic_pointer_cast<CAddNoise_Param>(m_pParam);

    if (pInput == nullptr || pParam == nullptr)
        throw CException(CoreExCode::INVALID_PARAMETER, "Invalid parameters", __func__, __FILE__, __LINE__);

    if (pInput->isDataAvailable() == false)
        throw CException(CoreExCode::INVALID_PARAMETER, "Empty image", __func__, __FILE__, __LINE__);

    CMat imgSrc = pInput->getImage();
    CMat imgDst(imgSrc.size(), imgSrc.type());
    //initialier l image imgDst à 0
    imgDst.setTo(cv::Scalar(0));

    emit m_signalHandler->doProgress();


    if(pParam->m_noiseType == "Gaussian")
    {
        try
        {
            cv::randn(imgDst, pParam->mean, pParam->sigma);
            imgDst = imgSrc + imgDst;
        }
        catch (cv::Exception &e)
        {
            throw CException(CoreExCode::INVALID_PARAMETER, e, __func__, __FILE__, __LINE__);
        }
    }
    else
    {
        try
        {
            double salt_noise_ratio = pParam->m_salt_p;
            double pepper_noise_ratio = pParam->m_pepper_p;

            //generate random numbers
            cv::RNG random_noise;

            //add salt and pepper noise
            for (int row = 0; row < imgDst.rows; ++row) {
                for (int col = 0; col < imgDst.cols; ++col) {

                    //generate random number between 0 and 1
                    float noise_probability = random_noise.uniform(0.0f, 1.0f);

                    //if probability is greater than 0.5, add salt noise
                    if (noise_probability < salt_noise_ratio) {
                        imgDst.at<cv::Vec3b>(row, col)[0] = 255;
                        imgDst.at<cv::Vec3b>(row, col)[1] = 255;
                        imgDst.at<cv::Vec3b>(row, col)[2] = 255;
                    }
                    //if probability is less than 0.5, add pepper noise
                    else if (noise_probability < pepper_noise_ratio) {
                        imgDst.at<cv::Vec3b>(row, col)[0] = 0;
                        imgDst.at<cv::Vec3b>(row, col)[1] = 0;
                        imgDst.at<cv::Vec3b>(row, col)[2] = 0;
                    }
                }
            }

            imgDst = imgSrc + imgDst;
        }
        catch (cv::Exception &e)
        {
            throw CException(CoreExCode::INVALID_PARAMETER, e, __func__, __FILE__, __LINE__);
        }
    }

    emit m_signalHandler->doProgress();

    auto pOutput = std::dynamic_pointer_cast<CImageIO>(getOutput(0));
    if (pOutput)
        pOutput->setImage(imgDst);

    endTaskRun();
    emit m_signalHandler;
}

//------------------------------
//----- C_PluginName_Widget
//------------------------------
CAddNoise_Widget::CAddNoise_Widget(QWidget *parent) : CWorkflowTaskWidget(parent)
{   
    // Déclaration de la variable m_pParam ici
    m_pParam = std::make_shared<CAddNoise_Param>();
    init();
}

CAddNoise_Widget::CAddNoise_Widget(std::shared_ptr<CWorkflowTaskParam> pParam, QWidget *parent): CWorkflowTaskWidget(parent)
{   
    // Create specific parameters object
    m_pParam = std::dynamic_pointer_cast<CAddNoise_Param>(pParam);
    // Initialize the widget
    init();
}

void CAddNoise_Widget::init()
{
    if (m_pParam == nullptr)
        m_pParam = std::make_shared<CAddNoise_Param>();

    auto pParam = std::dynamic_pointer_cast<CAddNoise_Param>(m_pParam);
    assert(pParam);


    m_pComboNoise = addCombo(tr("NoiseType"));
    m_pComboNoise->addItem("Gaussian");
    m_pComboNoise->addItem("Salt_Pepper");
    m_pComboNoise->setCurrentText(QString::fromStdString(pParam->m_noiseType));

    //gaussian noise parameters
    auto pSpinSigma = addDoubleSpin(tr("Sigma"), pParam->sigma, 0.0, 255, 1, 4);
    auto pSpinMean = addDoubleSpin(tr("Mean"), pParam->mean, 0.0, 255, 1, 4);
    //salt and pepper parameters
    auto pSpinSalt = addDoubleSpin(tr("salt probability"), pParam->m_salt_p, 0.0, 1, 0.01, 4);
    auto pSpinPepper = addDoubleSpin(tr("Pepper probability"), pParam->m_pepper_p, 0.0, 1, 0.01, 4);

    //connections
    connect(m_pComboNoise, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index)
        {
            Q_UNUSED(index);
            auto pParam = std::dynamic_pointer_cast<CAddNoise_Param>(m_pParam);
            assert(pParam);
            pParam->m_noiseType = m_pComboNoise->currentText().toStdString();

        });

    connect(pSpinSigma, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](double val)
        {
            auto pParam = std::dynamic_pointer_cast<CAddNoise_Param>(m_pParam);
            assert(pParam);
            pParam->sigma = val;
        });

    connect(pSpinMean, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](double val)
        {
            auto pParam = std::dynamic_pointer_cast<CAddNoise_Param>(m_pParam);
            assert(pParam);
            pParam->mean = val;
        });


    connect(pSpinSalt, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](double val)
        {
            auto pParam = std::dynamic_pointer_cast<CAddNoise_Param>(m_pParam);
            assert(pParam);
            pParam->m_salt_p = val;
        });

    connect(pSpinPepper, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](double val)
        {
            auto pParam = std::dynamic_pointer_cast<CAddNoise_Param>(m_pParam);
            assert(pParam);
            pParam->m_pepper_p = val;
        });


}

void CAddNoise_Widget::onApply()
{
    auto pParam = std::dynamic_pointer_cast<CAddNoise_Param>(m_pParam);
    emit doApplyProcess(m_pParam);

}
