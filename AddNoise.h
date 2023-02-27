#ifndef ADDNOISE_H
#define ADDNOISE_H

#include "CPluginProcessInterface.hpp"
#include "Workflow/CWorkflowTaskParam.h"
#include "Core/C2dImageTask.h"
#include "IO/CImageIO.h"
#include "Core/CWidgetFactory.hpp"
#include "Core/C2dImageTask.h"
#include "Workflow/CWorkflowTaskWidget.h"
#include "AddNoiseGlobal.h"

//------------------------------
//----- CAddNoise_Param
//------------------------------
class ADDNOISESHARED_EXPORT CAddNoise_Param: public CWorkflowTaskParam
{
    public:

        CAddNoise_Param();
        //Set parameters values from Ikomia application
        void        setParamMap(const UMapString& paramMap) override;

        //Send parameters values to Ikomia application
        UMapString  getParamMap() const override;
        
    public:
    std::string m_noiseType="Gaussian";
    double    sigma = 0;
    double    mean = 0;
    double    m_salt_p = 0;
    double    m_pepper_p = 0;

};

//------------------------------
//----- C_AddNoise
//------------------------------
class ADDNOISESHARED_EXPORT CAddNoise : public C2dImageTask
{
    public:

        CAddNoise();
        CAddNoise(const std::string& name, const std::shared_ptr<CAddNoise_Param>& pParam);

        //Function returning the number of progress steps frot this process
        //This is handled by the main progress bar of Ikomia application
        size_t  getProgressSteps();

        void run() override;
};

class ADDNOISESHARED_EXPORT CAddNoise_Factory : public CTaskFactory
{
    public:

        CAddNoise_Factory()
        {
            //Mandatory fields
            m_info.m_name = "AddNoise";
            m_info.m_description = "add different noise types to image like: salt&pepper noise, gaussian noise";
            m_info.m_authors = "";
            m_info.m_version = "1.0.0";
            //Relative path -> as displayed in Ikomia application process tree
            m_info.m_path = "Plugins";
            m_info.m_iconPath = "";
            //For search
            m_info.m_keywords = "salt and pepper noise, gaussian noise";
            m_info.m_language = ApiLanguage::CPP;
        }

        virtual WorkflowTaskPtr create(const WorkflowTaskParamPtr& pParam) override
        {
            auto pDerivedParam = std::dynamic_pointer_cast<CAddNoise_Param>(pParam);
            if(pDerivedParam != nullptr)
                return std::make_shared<CAddNoise>(m_info.m_name, pDerivedParam);
            else
                return create();
        }
        virtual WorkflowTaskPtr create() override
        {
            auto pDerivedParam = std::make_shared<CAddNoise_Param>();
            assert(pDerivedParam != nullptr);
            return std::make_shared<CAddNoise>(m_info.m_name, pDerivedParam);
        }
};

//------------------------------
//----- CAddNoise_Widget
//------------------------------
class ADDNOISESHARED_EXPORT CAddNoise_Widget : public CWorkflowTaskWidget
{
    public:

        CAddNoise_Widget(QWidget *parent = Q_NULLPTR);
        CAddNoise_Widget(WorkflowTaskParamPtr pParam, QWidget *parent = Q_NULLPTR);

        void onApply() override;

    private:
        std::shared_ptr<CAddNoise_Param> m_pParam;
        void init();

    private:
        QComboBox*          m_pComboNoise = nullptr;

};

class ADDNOISESHARED_EXPORT CAddNoise_WidgetFactory : public CWidgetFactory
{
    public:

        CAddNoise_WidgetFactory()
        {
            m_name = "AddNoise";
        }

        virtual WorkflowTaskWidgetPtr   create(WorkflowTaskParamPtr pParam)
        {
            return std::make_shared<CAddNoise_Widget>(pParam);
        }
};

//------------------------------
//----- Global plugin interface
//------------------------------
class ADDNOISESHARED_EXPORT CAddNoise_Interface : public QObject, public CPluginProcessInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "ikomia.plugin.process")
    Q_INTERFACES(CPluginProcessInterface)

    public:

        virtual std::shared_ptr<CTaskFactory> getProcessFactory()
        {
            return std::make_shared<CAddNoise_Factory>();
        }

        virtual std::shared_ptr<CWidgetFactory> getWidgetFactory()
        {
            return std::make_shared<CAddNoise_WidgetFactory>();
        }
};

#endif 
