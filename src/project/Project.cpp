#include "Project.h"
#include "../model/TuringMachine.h"
#include "../document/CodeDocument.h"
#include "../document/TapeDocument.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>
#include <QUuid>

Project::Project(const std::string& name)
    : m_name(name), m_isModified(false)
{
    // Create the Turing machine
    m_machine = std::make_unique<TuringMachine>(name);
    
    // Create the code document
    m_codeDocument = std::make_unique<CodeDocument>(this, "Code for " + name);
    
    // Create a default tape
    createTape("Default Tape");
}

Project::~Project()
{
}

std::string Project::getName() const
{
    return m_name;
}

void Project::setName(const std::string& name)
{
    if (m_name != name) {
        m_name = name;
        setModified(true);
        emit nameChanged(m_name);
    }
}

std::string Project::getFilePath() const
{
    return m_filePath;
}

void Project::setFilePath(const std::string& path)
{
    m_filePath = path;
}

bool Project::isModified() const
{
    return m_isModified;
}

void Project::setModified(bool modified)
{
    if (m_isModified != modified) {
        m_isModified = modified;
        emit modificationChanged(m_isModified);
    }
}

TapeDocument* Project::createTape(const std::string& name)
{
    std::string id = generateUniqueTapeId();
    m_tapeDocuments.push_back(std::make_unique<TapeDocument>(this, id, name));
    
    TapeDocument* tapeDoc = m_tapeDocuments.back().get();
    setModified(true);
    emit tapeAdded(tapeDoc);
    
    return tapeDoc;
}

TapeDocument* Project::getTape(const std::string& id) const
{
    for (const auto& tape : m_tapeDocuments) {
        if (tape->getId() == id) {
            return tape.get();
        }
    }
    return nullptr;
}

std::vector<TapeDocument*> Project::getAllTapes() const
{
    std::vector<TapeDocument*> tapes;
    for (const auto& tape : m_tapeDocuments) {
        tapes.push_back(tape.get());
    }
    return tapes;
}

bool Project::saveToFile(const std::string& path)
{
    QJsonObject projectJson;
    
    // Save project metadata
    projectJson["name"] = QString::fromStdString(m_name);
    projectJson["version"] = "1.0";
    
    // Save machine data
    projectJson["machine"] = QJsonObject::fromVariantMap({
        {"name", QString::fromStdString(m_machine->getName())},
        {"code", QString::fromStdString(m_machine->getOriginalCode())},
        {"machineData", QString::fromStdString(m_machine->toJson())}
    });
    
    // Save tapes
    QJsonArray tapesArray;
    for (const auto& tape : m_tapeDocuments) {
        QJsonObject tapeJson;
        tapeJson["id"] = QString::fromStdString(tape->getId());
        tapeJson["name"] = QString::fromStdString(tape->getName());
        tapeJson["content"] = QString::fromStdString(tape->getTape()->getCurrentContent());
        tapeJson["headPosition"] = tape->getTape()->getHeadPosition();
        tapesArray.append(tapeJson);
    }
    projectJson["tapes"] = tapesArray;
    
    // Write to file
    QJsonDocument doc(projectJson);
    QFile file(QString::fromStdString(path));
    
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for writing:" << QString::fromStdString(path);
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    
    // Update file path and reset modified flag
    setFilePath(path);
    setModified(false);
    
    return true;
}

std::unique_ptr<Project> Project::loadFromFile(const std::string& path)
{
    QFile file(QString::fromStdString(path));
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file for reading:" << QString::fromStdString(path);
        return nullptr;
    }
    
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid JSON in project file";
        return nullptr;
    }
    
    QJsonObject projectJson = doc.object();
    
    // Create a new project
    std::string projectName = "Untitled";
    if (projectJson.contains("name")) {
        projectName = projectJson["name"].toString().toStdString();
    }
    
    auto project = std::make_unique<Project>(projectName);
    
    // Load machine data
    if (projectJson.contains("machine") && projectJson["machine"].isObject()) {
        QJsonObject machineJson = projectJson["machine"].toObject();
        
        if (machineJson.contains("machineData")) {
            std::string machineData = machineJson["machineData"].toString().toStdString();
            try {
                // Create a new machine from JSON
                auto loadedMachine = TuringMachine::fromJson(machineData);
                if (loadedMachine) {
                    project->m_machine = std::move(loadedMachine);
                }
            } catch (const std::exception& e) {
                qWarning() << "Error loading machine data:" << e.what();
            }
        }
        
        // Set the code in the code document
        if (machineJson.contains("code")) {
            std::string code = machineJson["code"].toString().toStdString();
            project->m_machine->setOriginalCode(code);
            project->m_codeDocument->setCode(code);
        }
    }
    
    // Clear any default tapes
    project->m_tapeDocuments.clear();
    
    // Load tapes
    if (projectJson.contains("tapes") && projectJson["tapes"].isArray()) {
        QJsonArray tapesArray = projectJson["tapes"].toArray();
        
        for (const QJsonValue& tapeValue : tapesArray) {
            if (!tapeValue.isObject()) continue;
            
            QJsonObject tapeJson = tapeValue.toObject();
            
            std::string id = tapeJson["id"].toString().toStdString();
            std::string name = tapeJson["name"].toString().toStdString();
            
            // Create a new tape document
            auto tapeDoc = std::make_unique<TapeDocument>(project.get(), id, name);
            
            // Set content and head position
            if (tapeJson.contains("content")) {
                tapeDoc->getTape()->setInitialContent(tapeJson["content"].toString().toStdString());
            }
            
            if (tapeJson.contains("headPosition")) {
                tapeDoc->getTape()->setHeadPosition(tapeJson["headPosition"].toInt());
            }
            
            project->m_tapeDocuments.push_back(std::move(tapeDoc));
        }
    }
    
    // If no tapes were loaded, create a default one
    if (project->m_tapeDocuments.empty()) {
        project->createTape("Default Tape");
    }
    
    // Set the file path
    project->setFilePath(path);
    project->setModified(false);
    
    return project;
}

std::string Project::generateUniqueTapeId() const
{
    // Generate a unique ID for a tape using QUuid
    QString uuid = QUuid::createUuid().toString();
    
    // Remove curly braces from the UUID
    uuid.remove('{').remove('}');
    
    return "tape_" + uuid.toStdString();
}