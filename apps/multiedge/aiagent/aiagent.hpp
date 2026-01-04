#ifndef MULTIEDGE_AIAGENT_AIAGENT_HPP
#define MULTIEDGE_AIAGENT_AIAGENT_HPP
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
 *  \file        multiedge/aiagent/aiagent.hpp
 *  \ingroup     Areg Edge AI, Edge Device
 *  \author      Artak Avetyan
 *  \brief       Edge Device Dialog.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QDialog>

#include <QList>
#include "multiedge/resources/NEMultiEdge.hpp"
QT_BEGIN_NAMESPACE
namespace Ui {
class AIAgent;
}
QT_END_NAMESPACE

class AgentChatHistory;
class QPushButton;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPlainTextEdit;
class QTableView;
class QTabWidget;
class QWidget;

class AIAgent : public QDialog
{
    Q_OBJECT

public:
    AIAgent(QWidget *parent = nullptr);
    ~AIAgent();
    
    inline QString getActiveModelPath(void) const;

    uint32_t getTextLength(void) const;

    uint32_t getTokens(void) const;

    uint32_t getBatching(void) const;

    uint32_t getThreads(void) const;

    float getTemperature(void) const;

    float getProbability(void) const;

    void disconnectAgent(void);
    
public slots:
    
    void slotServiceStarted(bool isStarted);
    
    void slotAgentQueueSize(uint32_t queueSize);
    
    void slotActiveModelChanged(QString modelName);
    
    void slotAgentType(NEMultiEdge::eEdgeAgent EdgeAgent);

    void slotTextRequested(uint32_t seqId, uint32_t id, QString question, uint64_t stamp);
    
    void slotTextProcessed(uint32_t seqId, uint32_t id, QString reply, uint64_t stamp);
    
    void slotVideoProcessed(uint32_t seqId, uint32_t id, SharedBuffer video);
    
    void slotAgentProcessingFailed(NEMultiEdge::eEdgeAgent agent, NEService::eResultType reason);
    
private:

    inline QWidget* wndConnect(void) const;
    inline QWidget* wndChat(void) const;
    inline QPushButton* ctrlConnect(void) const;
    inline QLineEdit* ctrlAddress(void) const;
    inline QLineEdit* ctrlPort(void) const;
    inline QTableView* ctrlTable(void) const;
    inline QPushButton* ctrlClose(void) const;
    inline QTabWidget* ctrlTab(void) const;
    inline QListWidget* ctrlModels(void) const;
    inline QPushButton* ctrlActivate(void) const;
    inline QLineEdit* ctrlLocation(void) const;
    inline QPushButton* ctrlBrowse(void) const;
    inline QLineEdit* ctrlActiveModel(void) const;
    inline QPlainTextEdit* ctrlDisplay(void) const;
    
private slots:
    
    void onConnectClicked(bool checked);
    
    void onActivateClicked(bool clicked);
    
    void onModelLocationClicked(bool clicked);    
    
    void onModelsDoubleClicked(QListWidgetItem *item);
    
    void onModelsRowChanged(int currentRow);
    
    void onTableSelChanged(const QModelIndex &index);
    
private:
    void setupData(void);
    
    void setupWidgets(void);
    
    void setupSignals(void);
    
    bool routerConnect(void);
    
    void routerDisconnect(void);
    
    void setTemperature(float newTemp, float newMinP);
    
    QStringList scanTextLlamaModels(const QString& modelPath);

private:
    Ui::AIAgent*        ui;
    QString             mAddress;
    uint16_t            mPort;
    AgentChatHistory*   mModel;
    QString             mModelDir;
    QString             mAIModelName;
    QString             mAIModelPath;
};

inline QString AIAgent::getActiveModelPath(void) const
{
    return mAIModelPath;
}

#endif // MULTIEDGE_AIAGENT_AIAGENT_HPP
