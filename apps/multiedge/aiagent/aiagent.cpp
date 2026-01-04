/************************************************************************
 * This file is part of the Areg Edge AI project powered by AREG SDK.
 * The project contains multiple examples of using Edge AI based on Areg communication framework.
 *
 *  Areg Edge AI is available as free and open-source software under the MIT License.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2025 Aregtech UG. All rights reserved.
 *  \file        multiedge/aiagent/aiagent.cpp
 *  \ingroup     Areg Edge AI, Edge Device
 *  \author      Artak Avetyan
 *  \brief       Edge Device Dialog.
 *
 ************************************************************************/
#include "multiedge/aiagent/aiagent.hpp"
#include "ui/ui_AIAgent.h"

#include "areg/appbase/Application.hpp"
#include "areg/base/NEUtilities.hpp"
#include "areg/component/ComponentLoader.hpp"
#include "areg/component/NEService.hpp"
#include "areg/ipc/ConnectionConfiguration.hpp"
#include "multiedge/resources/nemultiedgesettings.hpp"
#include "multiedge/aiagent/agentprovider.hpp"
#include "multiedge/aiagent/agentchathistory.hpp"
#include "multiedge/aiagent/agentprocessor.hpp"

#include <QDir>
#include <QFileDialog>
#include <QListWidgetItem>
#include <QString>
#include <any>

BEGIN_MODEL(NEMultiEdgeSettings::MODEL_PROVIDER.data())
    BEGIN_REGISTER_THREAD(NEMultiEdgeSettings::AGENT_THREAD.data())
        BEGIN_REGISTER_COMPONENT(NEMultiEdgeSettings::SERVICE_PROVIDER.data(), AgentProvider)
            REGISTER_IMPLEMENT_SERVICE(NEMultiEdge::ServiceName, NEMultiEdge::InterfaceVersion)
            REGISTER_WORKER_THREAD(NEMultiEdgeSettings::WORKER_THREAD.data(), NEMultiEdgeSettings::CONSUMER_NAME.data())
        END_REGISTER_COMPONENT(NEMultiEdgeSettings::SERVICE_PROVIDER.data())
    END_REGISTER_THREAD(NEMultiEdgeSettings::AGENT_THREAD.data())
END_MODEL(NEMultiEdgeSettings::MODEL_PROVIDER.data())

AIAgent::AIAgent(QWidget *parent)
    : QDialog   (parent)
    , ui        (new Ui::AIAgent)
    , mAddress  (QString::fromStdString(NEMultiEdgeSettings::ROUTER_ADDRESS.data()))
    , mPort     (NEMultiEdgeSettings::ROUTER_PORT)
    , mModel    (nullptr)
    , mModelDir ( )
    , mAIModelName( )
    , mAIModelPath( )
{
    ui->setupUi(this);
    setupData();
    setupWidgets();
    setupSignals();
}

AIAgent::~AIAgent()
{
    routerDisconnect();
    delete ui;
}

void AIAgent::slotServiceStarted(bool isStarted)
{
}

void AIAgent::slotAgentQueueSize(uint32_t queueSize)
{
    ui->TxtQueueSize->setText(QString::number(queueSize));
}

void AIAgent::slotActiveModelChanged(QString modelName)
{
    ctrlActiveModel()->setText(modelName);
}

void AIAgent::slotAgentType(NEMultiEdge::eEdgeAgent EdgeAgent)
{
    const QString _agents[]
    {
          "Unknown"
        , "LLM"
        , "VLM"
        , "Hybrid"
    };
    
    ui->TxtAgentType->setText(_agents[static_cast<int>(EdgeAgent)]);
}

void AIAgent::slotTextRequested(uint32_t seqId, uint32_t id, QString question, uint64_t stamp)
{
    if (mModel != nullptr)
    {
        mModel->addRequest(question, id, seqId, stamp);
    }
}

void AIAgent::slotTextProcessed(uint32_t seqId, uint32_t id, QString reply, uint64_t stamp)
{
    if (mModel != nullptr)
    {
        mModel->addResponse(reply, id, seqId, stamp);
    }
}

void AIAgent::slotVideoProcessed(uint32_t seqId, uint32_t id, SharedBuffer video)
{
}

void AIAgent::slotAgentProcessingFailed(NEMultiEdge::eEdgeAgent agent, NEService::eResultType reason)
{
    if (mModel != nullptr)
    {
        QString text{NEMultiEdge::getString(agent)};
        text += ": Failed to process a request, reason = ";
        text += NEService::getString(reason);
        mModel->addFailure(text);
    }
}

inline QWidget* AIAgent::wndConnect(void) const
{
    return ui->WndConnect;
}

inline QWidget* AIAgent::wndChat(void) const
{
    return ui->WndChat;
}

inline QPushButton* AIAgent::ctrlConnect(void) const
{
    return ui->BtnConnect;
}

inline QLineEdit* AIAgent::ctrlAddress(void) const
{
    return ui->RouterAddress;
}

inline QLineEdit* AIAgent::ctrlPort(void) const
{
    return ui->RouterPort;
}

inline QTableView* AIAgent::ctrlTable(void) const
{
    return ui->TableHistory;
}

inline QPushButton* AIAgent::ctrlClose(void) const
{
    return ui->BtnClose;
}

inline QTabWidget* AIAgent::ctrlTab(void) const
{
    return ui->tabWidget;
}

inline QListWidget* AIAgent::ctrlModels(void) const
{
    return ui->ListModels;
}

inline QPushButton* AIAgent::ctrlActivate(void) const
{
    return ui->BtnActivate;
}

inline QLineEdit* AIAgent::ctrlLocation(void) const
{
    return ui->TxtModelDir;
}

inline QPushButton* AIAgent::ctrlBrowse(void) const
{
    return ui->BtnBrowse;
}

inline QLineEdit* AIAgent::ctrlActiveModel(void) const
{
    return ui->TxtActiveModel;
}

inline QPlainTextEdit* AIAgent::ctrlDisplay(void) const
{
    return ui->TxtDisplay;
}

void AIAgent::onActivateClicked(bool clicked)
{
    QListWidget* listModels = ctrlModels();
    onModelsDoubleClicked(listModels->currentItem());
}

void AIAgent::onModelLocationClicked(bool clicked)
{
    QFileDialog dlgFile(  this
                        , QString(tr("Select AI Model Directory"))
                        , mModelDir
                        , QString(""));
    dlgFile.setLabelText(QFileDialog::DialogLabel::FileName, QString(tr("AI Model Location:")));
    
    
    dlgFile.setOptions(QFileDialog::Option::ShowDirsOnly);
    dlgFile.setFileMode(QFileDialog::Directory);
    if (mModelDir.isEmpty())
    {
        QDir curDir(QDir::current());
        dlgFile.setDirectory(curDir.exists() ? curDir.absolutePath() : QString());
    }
    else
    {
        dlgFile.setDirectory(mModelDir);
    }
    
    if (dlgFile.exec() == static_cast<int>(QDialog::DialogCode::Accepted))
    {
        QStringList models{ scanTextLlamaModels(dlgFile.directory().path()) };
        if (models.isEmpty() == false)
        {
            QListWidget* listModels = ctrlModels();
            listModels->clear();
            listModels->addItems(models);
            listModels->setCurrentRow(-1);
            QList<QListWidgetItem*> items = listModels->findItems(mAIModelName, Qt::MatchExactly);
            if (items.isEmpty() == false)
            {
                listModels->setCurrentItem(items[0]);
            }
        }
    }
}

void AIAgent::onModelsDoubleClicked(QListWidgetItem *item)
{
    if (item == nullptr)
        return;
    
    QString modelName = item->text();
    if (modelName.isEmpty())
        return;
    
    QFileInfo fi(mModelDir, modelName);
    if (fi.exists())
    {
        mAIModelName = modelName;
        mAIModelPath = fi.absoluteFilePath();
        AgentProvider::activateModel(mAIModelPath);
    }
}

void AIAgent::onModelsRowChanged(int currentRow)
{
    Q_ASSERT(ctrlModels() != nullptr);
    int rows = ctrlModels()->count();
    QListWidgetItem * item { nullptr };
    if (currentRow >= 0)
    {
        if (currentRow < rows)
        {
            item = ctrlModels()->item(currentRow);
        }
    }
    
    QString modelName = item != nullptr ? item->text() : "";
    ctrlActivate()->setEnabled(modelName.isEmpty() == false);
}

void AIAgent::onTableSelChanged(const QModelIndex &index)
{
    if (index.isValid() == false)
        return;

    const QString& msg = mModel->getRowMessage(index.row());
    ctrlDisplay()->setPlainText(msg);
}

void AIAgent::setupData(void)
{
    ConnectionConfiguration config(NERemoteService::eRemoteServices::ServiceRouter, NERemoteService::eConnectionTypes::ConnectTcpip);
    if (config.isConfigured())
    {
        mPort   = static_cast<uint16_t>(config.getConnectionPort());
        mAddress= config.getConnectionAddress();
    }
    
    String name = NEUtilities::generateName(NEMultiEdgeSettings::SERVICE_CONSUMER.data());
    ctrlAddress()->setText(mAddress);
    ctrlPort()->setText(QString::number(mPort));
    ui->TxtQueueSize->setText("N/A");
    ui->TxtAgentType->setText("N/A");
    ui->TxtModelDir->setText("N/A");
    
    ui->TxtLength->setValidator(    new QIntValidator(AgentProcessor::MIN_CHARS   , AgentProcessor::MAX_CHARS         , this));
    ui->TxtTokens->setValidator(    new QIntValidator(AgentProcessor::MIN_TOKENS  , AgentProcessor::MAX_TOKENS        , this));
    ui->TxtBatching->setValidator(  new QIntValidator(AgentProcessor::MIN_BATCHING, AgentProcessor::MAX_BATCHING      , this));
    ui->TxtThreads->setValidator(   new QIntValidator(AgentProcessor::MIN_THREADS , AgentProcessor::optThreadCount()  , this));
    
    ui->TxtLength->setText(QString::number(AgentProcessor::DEF_CHARS));
    ui->TxtTokens->setText(QString::number(AgentProcessor::DEF_TOKENS));
    ui->TxtBatching->setText(QString::number(AgentProcessor::DEF_BATCHING));
    ui->TxtThreads->setText(QString::number(AgentProcessor::defThreadCount()));
    
    mModel = new AgentChatHistory(this);
    ctrlTable()->setModel(mModel);
}

void AIAgent::setupWidgets(void)
{
    QIcon icon(":/icons/icon-edge-ai");
    setWindowIcon(icon);
    
    // Ensure the header is explicitly shown. Designer settings / style sheets can keep it hidden,
    // and changing resize mode on a hidden header has no visible effect.
    QTableView* table = ctrlTable();
    ASSERT(table != nullptr);
    table->setCornerButtonEnabled(false);
        
    if (QHeaderView* header = table->horizontalHeader())
    {
        header->setVisible(true);
        header->setHighlightSections(false);
        header->setSectionsClickable(true);
        header->setStretchLastSection(true);
        header->setSectionResizeMode(QHeaderView::Interactive);
        header->setSectionResizeMode(0, QHeaderView::ResizeMode::ResizeToContents);
        header->setSectionResizeMode(1, QHeaderView::ResizeMode::Interactive);
        header->setSectionResizeMode(2, QHeaderView::ResizeMode::Interactive);
        header->setSectionResizeMode(3, QHeaderView::ResizeMode::Interactive);
    }
        
    table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        
    // Make sure the view has some header height calculated and repaints with updated header state.
    table->updateGeometry();
    table->viewport()->update();
    
    ctrlActiveModel()->setText("N/A");
    QListWidget* listModels = ctrlModels();
    listModels->clear();
    QStringList list = scanTextLlamaModels(QString());
    ctrlLocation()->setText(mModelDir);
    listModels->addItems(list);
    if (list.isEmpty() == false)
    {
        listModels->setCurrentRow(0);
        mAIModelName = list[0];
        ctrlConnect()->setEnabled(true);
        QFileInfo fi(mModelDir, mAIModelName);
        mAIModelPath = fi.absoluteFilePath();
    }
    else
    {
        ctrlConnect()->setEnabled(false);
    }
    
    ui->BtnPrecise->setChecked(true);
    ctrlTab()->setCurrentIndex(0);
    
    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::WindowMinimizeButtonHint;
    flags |= Qt::WindowSystemMenuHint;
    setWindowFlags(flags);
}

void AIAgent::setupSignals(void)
{
    connect(ctrlClose()     , &QPushButton::clicked, this, [this](bool checked) {routerDisconnect(); close();});
    connect(ctrlConnect()   , &QPushButton::clicked, this, &AIAgent::onConnectClicked);
    connect(ctrlActivate()  , &QPushButton::clicked, this, &AIAgent::onActivateClicked);
    connect(ctrlBrowse()    , &QPushButton::clicked, this, &AIAgent::onModelLocationClicked);
    connect(ctrlModels()    , &QListWidget::itemDoubleClicked, this, &AIAgent::onModelsDoubleClicked);
    connect(ctrlModels()    , &QListWidget::currentRowChanged, this, &AIAgent::onModelsRowChanged);
    connect(ctrlTable()     , &QTableView::activated        , this , &AIAgent::onTableSelChanged);
    connect(ctrlTable()     , &QTableView::doubleClicked    , this , &AIAgent::onTableSelChanged);
    connect(ui->BtnAnswer   , &QRadioButton::toggled, this, [this](bool checked){if (checked) setTemperature(0.00f, 0.00f);});
    connect(ui->BtnPrecise  , &QRadioButton::toggled, this, [this](bool checked){if (checked) setTemperature(0.10f, 0.12f);});
    connect(ui->BtnBalanced , &QRadioButton::toggled, this, [this](bool checked){if (checked) setTemperature(0.30f, 0.10f);});
    connect(ui->BtnConvers  , &QRadioButton::toggled, this, [this](bool checked){if (checked) setTemperature(0.50f, 0.08f);});
    connect(ui->BtnCreative , &QRadioButton::toggled, this, [this](bool checked){if (checked) setTemperature(0.75f, 0.06f);});
    connect(ui->BtnExperim  , &QRadioButton::toggled, this, [this](bool checked){if (checked) setTemperature(1.00f, 0.05f);});
}

bool AIAgent::routerConnect(void)
{
    mAddress= ctrlAddress()->text();
    mPort   = static_cast<uint16_t>(ctrlPort()->text().toUInt());
    
    ConnectionConfiguration config(NERemoteService::eRemoteServices::ServiceRouter, NERemoteService::eConnectionTypes::ConnectTcpip);
    if (config.isConfigured())
    {
        config.setConnectionAddress(mAddress.toStdString());
        config.setConnectionPort(mPort);
        if (Application::startMessageRouting(mAddress.toStdString().c_str(), mPort))
        {
            mModel->resetHistory();
            ctrlTab()->setCurrentIndex(1);
            if (ComponentLoader::setComponentData(NEMultiEdgeSettings::SERVICE_PROVIDER.data(), std::make_any<AIAgent *>(this)))
            {
                QListWidgetItem * item = ctrlModels()->currentItem();
                if (item != nullptr)
                {
                    mAIModelName = item->text();
                }
                
                QFileInfo fi(mModelDir, mAIModelName);
                mAIModelPath = fi.exists() ? fi.absoluteFilePath() : QString();
                return Application::loadModel(NEMultiEdgeSettings::MODEL_PROVIDER.data());
            }
        }
    }
    
    return false;
}

void AIAgent::routerDisconnect(void)
{
    Application::unloadModel(nullptr);
    Application::stopMessageRouting();
}

void AIAgent::onConnectClicked(bool checked)
{
    if ((Application::isRouterConnected() == false) && (Application::isRouterConnectionPending() == false))
    {
        if (routerConnect())
        {
            ctrlAddress()->setEnabled(false);
            ctrlPort()->setEnabled(false);
            ctrlConnect()->setText(tr("&Disconnect"));
            ctrlConnect()->setIcon(QIcon::fromTheme(QString::fromUtf8("network-offline")));
            ctrlConnect()->setShortcut(QCoreApplication::translate("AIAgent", "Alt+D", nullptr));
        }
        else
        {
            routerDisconnect();
            ctrlConnect()->setChecked(false);
        }
    }
    else
    {
        routerDisconnect();
        ctrlAddress()->setEnabled(true);
        ctrlPort()->setEnabled(true);
        ctrlConnect()->setText(tr("&Connect"));
        ctrlConnect()->setIcon(QIcon::fromTheme(QString::fromUtf8("network-wireless")));
        ctrlConnect()->setShortcut(QCoreApplication::translate("AIAgent", "Alt+C", nullptr));
    }
}

QStringList AIAgent::scanTextLlamaModels(const QString& modelPath)
{
    if (modelPath.isEmpty())
    {
        // Directory is relative to the current working directory (QDir::current()).
        constexpr const char ModelsRelPath[]{ "models/llama/text" };
        QDir dir(QDir::current().filePath(QString::fromUtf8(ModelsRelPath)));
        return scanTextLlamaModels(dir.absolutePath());
    }
    else
    {
        QDir dir(modelPath);
        if (dir.exists() == false)
            return QStringList{};
        
        mModelDir = dir.absolutePath();
        dir.setFilter(QDir::Files | QDir::Readable | QDir::NoDotAndDotDot);
        dir.setSorting(QDir::Name | QDir::IgnoreCase);
        
        // Return file names only, e.g. "model.gguf"
        return dir.entryList(QStringList{ QString::fromUtf8("*.gguf") }, QDir::Files, QDir::Name | QDir::IgnoreCase);
    }
}

void AIAgent::setTemperature(float newTemp, float newMinP)
{
    AgentProvider::setTemperature(newTemp, newMinP);
}

float AIAgent::getTemperature(void) const
{
    if (ui->BtnAnswer->isChecked())
        return 0.00f;
    else if (ui->BtnPrecise->isChecked())
        return 0.10f;
    else if (ui->BtnBalanced->isChecked())
        return 0.30f;
    else if (ui->BtnConvers->isChecked())
        return 0.50f;
    else if (ui->BtnCreative->isChecked())
        return 0.75f;
    else if (ui->BtnExperim->isChecked())
        return 1.00f;

    return 0.50f;
}

float AIAgent::getProbability(void) const
{
    if (ui->BtnAnswer->isChecked())
        return 0.00f;
    else if (ui->BtnPrecise->isChecked())
        return 0.12f;
    else if (ui->BtnBalanced->isChecked())
        return 0.10f;
    else if (ui->BtnConvers->isChecked())
        return 0.08f;
    else if (ui->BtnCreative->isChecked())
        return 0.06f;
    else if (ui->BtnExperim->isChecked())
        return 0.05f;

    return 0.50f;
}

void AIAgent::disconnectAgent(void)
{
    routerDisconnect();
}

uint32_t AIAgent::getTextLength(void) const
{
    bool ok{false};
    uint32_t res = ui->TxtLength->text().toUInt(&ok);
    if (ok)
    {
        return res;
    }
    else
    {
        ui->TxtLength->setText(QString::number(AgentProcessor::DEF_CHARS));
        return AgentProcessor::DEF_CHARS;
    }
}

uint32_t AIAgent::getTokens(void) const
{
    bool ok{false};
    uint32_t res = ui->TxtTokens->text().toUInt(&ok);
    if (ok)
    {
        return res;
    }
    else
    {
        ui->TxtTokens->setText(QString::number(AgentProcessor::DEF_TOKENS));
        return AgentProcessor::DEF_TOKENS;
    }
}

uint32_t AIAgent::getBatching(void) const
{
    bool ok{false};
    uint32_t res = ui->TxtBatching->text().toUInt();
    if (ok)
    {
        return res;
    }
    else
    {
        ui->TxtBatching->setText(QString::number(AgentProcessor::DEF_BATCHING));
        return AgentProcessor::DEF_BATCHING;
    }
}

uint32_t AIAgent::getThreads(void) const
{
    bool ok{false};
    uint32_t res = ui->TxtThreads->text().toUInt();
    if (ok)
    {
        return res;
    }
    else
    {
        res = AgentProcessor::defThreadCount();
        ui->TxtThreads->setText(QString::number(res));
        return res;
    }
}
