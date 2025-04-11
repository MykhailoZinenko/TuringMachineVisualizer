#pragma once

#include <memory>
#include <vector>
#include <string>
#include <QObject>

class Project;
class Document;

/**
 * Singleton class that manages all open projects
 */
class ProjectManager : public QObject
{
    Q_OBJECT

public:
    static ProjectManager& getInstance();

    // Project operations
    Project* createProject(const std::string& name = "Untitled");
    Project* openProject(const std::string& path);
    bool closeProject(Project* project);
    bool saveProject(Project* project);
    bool saveProjectAs(Project* project, const std::string& path);

    // Project access
    std::vector<Project*> getAllProjects() const;
    Project* findProjectByPath(const std::string& path) const;

    signals:
        void projectCreated(Project* project);
    void projectOpened(Project* project);
    void projectClosed(Project* project);
    void projectSaved(Project* project);

    public slots:
        void onDocumentClosed(Document* document);

private:
    ProjectManager();
    ~ProjectManager();

    // Prevent copying
    ProjectManager(const ProjectManager&) = delete;
    ProjectManager& operator=(const ProjectManager&) = delete;

    std::vector<std::unique_ptr<Project>> m_projects;
};