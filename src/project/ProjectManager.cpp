#include "ProjectManager.h"
#include "Project.h"
#include "../document/Document.h"
#include <QFileInfo>
#include <QDebug>

ProjectManager& ProjectManager::getInstance()
{
    static ProjectManager instance;
    return instance;
}

ProjectManager::ProjectManager()
{
}

ProjectManager::~ProjectManager()
{
}

Project* ProjectManager::createProject(const std::string& name)
{
    auto project = std::make_unique<Project>(name);
    Project* projectPtr = project.get();
    
    m_projects.push_back(std::move(project));
    emit projectCreated(projectPtr);
    
    return projectPtr;
}

Project* ProjectManager::openProject(const std::string& path)
{
    // Check if already open
    Project* existing = findProjectByPath(path);
    if (existing) {
        return existing;
    }
    
    // Try to load the project
    auto project = Project::loadFromFile(path);
    if (!project) {
        return nullptr;
    }
    
    Project* projectPtr = project.get();
    m_projects.push_back(std::move(project));
    emit projectOpened(projectPtr);
    
    return projectPtr;
}

bool ProjectManager::closeProject(Project* project)
{
    if (!project) return false;
    
    auto it = std::find_if(m_projects.begin(), m_projects.end(),
                          [project](const auto& p) { return p.get() == project; });
    
    if (it != m_projects.end()) {
        emit projectClosed(project);
        m_projects.erase(it);
        return true;
    }
    
    return false;
}

bool ProjectManager::saveProject(Project* project)
{
    if (!project) return false;
    
    if (project->getFilePath().empty()) {
        return false;  // Need a path
    }
    
    bool success = project->saveToFile(project->getFilePath());
    if (success) {
        emit projectSaved(project);
    }
    
    return success;
}

bool ProjectManager::saveProjectAs(Project* project, const std::string& path)
{
    if (!project) return false;
    
    bool success = project->saveToFile(path);
    if (success) {
        emit projectSaved(project);
    }
    
    return success;
}

std::vector<Project*> ProjectManager::getAllProjects() const
{
    std::vector<Project*> result;
    for (const auto& project : m_projects) {
        result.push_back(project.get());
    }
    return result;
}

Project* ProjectManager::findProjectByPath(const std::string& path) const
{
    for (const auto& project : m_projects) {
        if (project->getFilePath() == path) {
            return project.get();
        }
    }
    return nullptr;
}

void ProjectManager::onDocumentClosed(Document* document)
{
    if (!document) return;
    
    // Get the document's project
    Project* project = document->getProject();
    if (!project) return;
    
    // Check if all documents for this project are closed
    bool allClosed = true;
    
    // This would depend on how you track which documents are open
    // For now, we'll assume that a document being closed means it's no longer visible
    // in any tab. In practice, you'd need to integrate this with your tab system.
    
    if (allClosed) {
        closeProject(project);
    }
}