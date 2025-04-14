#include "SessionManager.h"
#include "../document/CodeDocument.h"
#include "../document/TapeDocument.h"
#include "../model/TuringMachine.h"
#include <QDebug>

SessionManager& SessionManager::getInstance()
{
    static SessionManager instance;
    return instance;
}

SessionManager::SessionManager()
    : m_activeCodeDocument(nullptr), m_activeTapeDocument(nullptr)
{
    qDebug() << "SessionManager created";
}

SessionManager::~SessionManager()
{
    qDebug() << "SessionManager destroyed";
}

void SessionManager::setActiveCodeDocument(CodeDocument* document)
{
    if (m_activeCodeDocument != document) {
        m_activeCodeDocument = document;
        emit activeCodeDocumentChanged(document);
        
        // If we have an active code document and it has a machine,
        // notify listeners that the active machine may have changed
        if (m_activeCodeDocument && m_activeCodeDocument->getMachine()) {
            emit activeMachineUpdated(m_activeCodeDocument->getMachine());
        }
    }
}

void SessionManager::setActiveTapeDocument(TapeDocument* document)
{
    if (m_activeTapeDocument != document) {
        m_activeTapeDocument = document;
        emit activeTapeDocumentChanged(document);

        // If we have a tape document and an active machine, notify listeners
        // This ensures code is applied when a new tape becomes active
        if (m_activeTapeDocument && m_activeCodeDocument && m_activeCodeDocument->getMachine()) {
            emit activeMachineUpdated(m_activeCodeDocument->getMachine());
        }
    }
}

CodeDocument* SessionManager::getActiveCodeDocument() const
{
    return m_activeCodeDocument;
}

TapeDocument* SessionManager::getActiveTapeDocument() const
{
    return m_activeTapeDocument;
}

TuringMachine* SessionManager::getActiveMachine() const
{
    if (m_activeCodeDocument) {
        return m_activeCodeDocument->getMachine();
    }
    return nullptr;
}