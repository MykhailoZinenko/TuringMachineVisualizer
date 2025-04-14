#include "FileListManager.h"
#include "../document/Document.h"
#include "../document/CodeDocument.h"
#include "../document/TapeDocument.h"

#include <QListWidget>
#include <QGroupBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QSettings>
#include <QFileInfo>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMessageBox>
#include <QInputDialog>
#include <QIcon>
#include <QDebug>

#include "StyleKit.h"

FileListManager::FileListManager(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    setupDragDrop();
    loadFileListsFromSettings();
}

FileListManager::~FileListManager()
{
    saveFileListsToSettings();
}

void FileListManager::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Create the accordions
    m_codeFilesGroup = createCodeFilesAccordion();
    m_tapeFilesGroup = createTapeFilesAccordion();
    
    // Add to layout
    mainLayout->addWidget(m_codeFilesGroup);
    mainLayout->addWidget(m_tapeFilesGroup);
    mainLayout->addStretch();
    
    // Set up connections
    connect(m_codeFilesList, &QListWidget::itemClicked, 
            this, &FileListManager::onCodeFileSelected);
    connect(m_tapeFilesList, &QListWidget::itemClicked, 
            this, &FileListManager::onTapeFileSelected);
    
    connect(m_codeFilesList, &QListWidget::itemDoubleClicked, 
            this, &FileListManager::onCodeFileDoubleClicked);
    connect(m_tapeFilesList, &QListWidget::itemDoubleClicked, 
            this, &FileListManager::onTapeFileDoubleClicked);
    
    connect(m_newCodeButton, &QPushButton::clicked, 
            this, &FileListManager::createNewCodeFile);
    connect(m_openCodeButton, &QPushButton::clicked, 
            this, &FileListManager::openCodeFile);
    connect(m_removeCodeButton, &QPushButton::clicked, 
            [this]() { removeSelectedFile(true); });
    
    connect(m_newTapeButton, &QPushButton::clicked, 
            this, &FileListManager::createNewTapeFile);
    connect(m_openTapeButton, &QPushButton::clicked, 
            this, &FileListManager::openTapeFile);
    connect(m_removeTapeButton, &QPushButton::clicked, 
            [this]() { removeSelectedFile(false); });
    
    // Initial state
    m_removeCodeButton->setEnabled(false);
    m_removeTapeButton->setEnabled(false);
}

QGroupBox* FileListManager::createCodeFilesAccordion()
{
    QGroupBox* groupBox = new QGroupBox(tr("Code Files"), this);
    groupBox->setCheckable(true);
    groupBox->setChecked(true);
    groupBox->setStyleSheet(StyleKit::getGroupBoxStyle());

    QVBoxLayout* layout = new QVBoxLayout(groupBox);
    layout->setSpacing(8);
    layout->setContentsMargins(10, 15, 10, 10); // Adjust for the title spacing

    // Create list widget with modern styling
    m_codeFilesList = new QListWidget(groupBox);
    m_codeFilesList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_codeFilesList->setDragDropMode(QAbstractItemView::DragDrop);
    m_codeFilesList->setAcceptDrops(true);
    m_codeFilesList->setAlternatingRowColors(true);
    m_codeFilesList->setStyleSheet(
        "QListWidget {"
        "  background-color: #FFFFFF;"
        "  border: 1px solid #BDBDBD;"
        "  border-radius: 3px;"
        "  padding: 4px;"
        "}"
        "QListWidget::item {"
        "  border-radius: 3px;"
        "  padding: 6px 8px;"
        "  margin: 3px 0px;"
        "  background-color: #F8F9FA;"      // Light background for all items
        "}"
        "QListWidget::item:selected {"
        "  background-color: #DBEAFE;"      // Distinct blue background for selected
        "  color: #1E40AF;"                 // Dark blue text for selected
        "  border: 1px solid #93C5FD;"      // Light blue border for selected
        "  font-weight: bold;"              // Bold text for selected items
        "}"
        "QListWidget::item:hover:!selected {"
        "  background-color: #F3F4F6;"      // Very light gray hover effect
        "  border: 1px solid #E5E7EB;"      // Subtle border on hover
        "}"
        "QListWidget::item:alternate {"
        "  background-color: #FAFAFA;"      // Extremely light gray for alternating rows
        "}"
    );

    layout->addWidget(m_codeFilesList);

    // Create buttons with modern styling
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);

    m_newCodeButton = new QPushButton(tr("New"), groupBox);
    m_openCodeButton = new QPushButton(tr("Open"), groupBox);
    m_removeCodeButton = new QPushButton(tr("Remove"), groupBox);

    // Apply button styles
    m_newCodeButton->setStyleSheet(StyleKit::getButtonStyle(true));  // Primary
    m_openCodeButton->setStyleSheet(StyleKit::getButtonStyle(false)); // Secondary
    m_removeCodeButton->setStyleSheet(StyleKit::getButtonStyle(false)); // Secondary

    buttonLayout->addWidget(m_newCodeButton);
    buttonLayout->addWidget(m_openCodeButton);
    buttonLayout->addWidget(m_removeCodeButton);

    layout->addLayout(buttonLayout);

    return groupBox;
}

QGroupBox* FileListManager::createTapeFilesAccordion()
{
    QGroupBox* groupBox = new QGroupBox(tr("Tape Files"), this);
    groupBox->setCheckable(true);
    groupBox->setChecked(true);
    groupBox->setStyleSheet(StyleKit::getGroupBoxStyle());

    QVBoxLayout* layout = new QVBoxLayout(groupBox);
    layout->setSpacing(8);
    layout->setContentsMargins(10, 15, 10, 10); // Adjust for the title spacing

    // Create list widget with modern styling
    m_tapeFilesList = new QListWidget(groupBox);
    m_tapeFilesList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tapeFilesList->setDragDropMode(QAbstractItemView::DragDrop);
    m_tapeFilesList->setAcceptDrops(true);
    m_tapeFilesList->setAlternatingRowColors(true);
    m_tapeFilesList->setStyleSheet(
        "QListWidget {"
        "  background-color: #FFFFFF;"
        "  border: 1px solid #BDBDBD;"
        "  border-radius: 3px;"
        "  padding: 4px;"
        "}"
        "QListWidget::item {"
        "  border-radius: 3px;"
        "  padding: 6px 8px;"
        "  margin: 3px 0px;"
        "  background-color: #F8F9FA;"      // Light background for all items
        "}"
        "QListWidget::item:selected {"
        "  background-color: #DCFCE7;"      // Light green background for selected
        "  color: #166534;"                 // Dark green text for selected
        "  border: 1px solid #86EFAC;"      // Light green border for selected
        "  font-weight: bold;"              // Bold text for selected items
        "}"
        "QListWidget::item:hover:!selected {"
        "  background-color: #F3F4F6;"      // Very light gray hover effect
        "  border: 1px solid #E5E7EB;"      // Subtle border on hover
        "}"
        "QListWidget::item:alternate {"
        "  background-color: #FAFAFA;"      // Extremely light gray for alternating rows
        "}"
    );

    layout->addWidget(m_tapeFilesList);

    // Create buttons with modern styling
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);

    m_newTapeButton = new QPushButton(tr("New"), groupBox);
    m_openTapeButton = new QPushButton(tr("Open"), groupBox);
    m_removeTapeButton = new QPushButton(tr("Remove"), groupBox);

    // Apply button styles
    m_newTapeButton->setStyleSheet(StyleKit::getButtonStyle(true));   // Primary
    m_openTapeButton->setStyleSheet(StyleKit::getButtonStyle(false)); // Secondary
    m_removeTapeButton->setStyleSheet(StyleKit::getButtonStyle(false)); // Secondary

    buttonLayout->addWidget(m_newTapeButton);
    buttonLayout->addWidget(m_openTapeButton);
    buttonLayout->addWidget(m_removeTapeButton);

    layout->addLayout(buttonLayout);

    return groupBox;
}

void FileListManager::setupDragDrop()
{
    setAcceptDrops(true);
}

bool FileListManager::addCodeFile(const QString& path)
{
    QFileInfo fileInfo(path);
    if (!fileInfo.exists() || !fileInfo.isFile() || 
        !fileInfo.suffix().toLower().endsWith("tm")) {
        return false;
    }
    
    // Check if already in list
    if (m_codeItems.contains(path)) {
        // Just select it
        QListWidgetItem* item = m_codeItems[path];
        m_codeFilesList->setCurrentItem(item);
        return true;
    }
    
    // Add to list
    QListWidgetItem* item = new QListWidgetItem(getFileIcon(path),
                                              fileInfo.fileName());
    item->setData(Qt::UserRole, path);
    item->setToolTip(path);
    
    m_codeFilesList->addItem(item);
    m_codeItems[path] = item;
    
    return true;
}

bool FileListManager::addTapeFile(const QString& path)
{
    QFileInfo fileInfo(path);
    if (!fileInfo.exists() || !fileInfo.isFile() || 
        !fileInfo.suffix().toLower().endsWith("tmt")) {
        return false;
    }
    
    // Check if already in list
    if (m_tapeItems.contains(path)) {
        // Just select it
        QListWidgetItem* item = m_tapeItems[path];
        m_tapeFilesList->setCurrentItem(item);
        return true;
    }
    
    // Add to list
    QListWidgetItem* item = new QListWidgetItem(getFileIcon(path),
                                              fileInfo.fileName());
    item->setData(Qt::UserRole, path);
    item->setToolTip(path);
    
    m_tapeFilesList->addItem(item);
    m_tapeItems[path] = item;
    
    return true;
}

QStringList FileListManager::getCodeFiles() const
{
    return m_codeItems.keys();
}

QStringList FileListManager::getTapeFiles() const
{
    return m_tapeItems.keys();
}

void FileListManager::saveFileListsToSettings()
{
    QSettings settings;
    settings.beginGroup("FileListManager");
    
    settings.setValue("codeFiles", getCodeFiles());
    settings.setValue("tapeFiles", getTapeFiles());
    
    settings.endGroup();
}

void FileListManager::loadFileListsFromSettings()
{
    QSettings settings;
    settings.beginGroup("FileListManager");
    
    QStringList codeFiles = settings.value("codeFiles").toStringList();
    for (const QString& path : codeFiles) {
        addCodeFile(path);
    }
    
    QStringList tapeFiles = settings.value("tapeFiles").toStringList();
    for (const QString& path : tapeFiles) {
        addTapeFile(path);
    }
    
    settings.endGroup();
}

void FileListManager::setActiveDocument(Document* document)
{
    if (!document) {
        m_activeDocumentPath.clear();
        updateSelection(QString());
        return;
    }
    
    QString filePath = QString::fromStdString(document->getFilePath());
    m_activeDocumentPath = filePath;
    updateSelection(filePath);
}

void FileListManager::updateSelection(const QString& path)
{
    // Update code list selection
    QListWidgetItem* codeItem = m_codeItems.value(path, nullptr);
    if (codeItem) {
        m_codeFilesList->setCurrentItem(codeItem);
        m_removeCodeButton->setEnabled(true);
    } else {
        m_codeFilesList->clearSelection();
        m_removeCodeButton->setEnabled(false);
    }
    
    // Update tape list selection
    QListWidgetItem* tapeItem = m_tapeItems.value(path, nullptr);
    if (tapeItem) {
        m_tapeFilesList->setCurrentItem(tapeItem);
        m_removeTapeButton->setEnabled(true);
    } else {
        m_tapeFilesList->clearSelection();
        m_removeTapeButton->setEnabled(false);
    }
}

QIcon FileListManager::getFileIcon(const QString& filePath)
{
    if (filePath.toLower().endsWith(".tm")) {
        // Code file icon
        return QIcon::fromTheme("text-x-script");
    } else if (filePath.toLower().endsWith(".tmt")) {
        // Tape file icon
        return QIcon::fromTheme("media-tape");
    } else {
        // Default icon
        return QIcon::fromTheme("text-x-generic");
    }
}

void FileListManager::createNewCodeFile()
{
    bool ok;
    QString name = QInputDialog::getText(this, tr("New Code File"),
                                      tr("Enter a name for the new code file:"),
                                      QLineEdit::Normal, "untitled.tm", &ok);
    if (!ok || name.isEmpty()) {
        return;
    }
    
    // Ensure it has .tm extension
    if (!name.toLower().endsWith(".tm")) {
        name += ".tm";
    }
    
    // Create a new document
    auto document = std::make_unique<CodeDocument>(name.toStdString());
    
    // Show save dialog to get file path
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Code File"),
                                                 name, tr("Turing Machine Files (*.tm)"));
    if (filePath.isEmpty()) {
        return;
    }
    
    // Ensure it has .tm extension
    if (!filePath.toLower().endsWith(".tm")) {
        filePath += ".tm";
    }
    
    // Save the document
    document->setFilePath(filePath.toStdString());
    if (!document->save()) {
        QMessageBox::warning(this, tr("Save Error"),
                            tr("Failed to save the code file."));
        return;
    }
    
    // Add to list
    addCodeFile(filePath);
    
    // Emit signal
    emit codeDocumentCreated(document.release());
    emit codeFileSelected(filePath);
}

void FileListManager::createNewTapeFile()
{
    bool ok;
    QString name = QInputDialog::getText(this, tr("New Tape File"),
                                      tr("Enter a name for the new tape file:"),
                                      QLineEdit::Normal, "untitled.tmt", &ok);
    if (!ok || name.isEmpty()) {
        return;
    }
    
    // Ensure it has .tmt extension
    if (!name.toLower().endsWith(".tmt")) {
        name += ".tmt";
    }
    
    // Create a new document
    auto document = std::make_unique<TapeDocument>(name.toStdString());
    
    // Show save dialog to get file path
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Tape File"),
                                                 name, tr("Tape Files (*.tmt)"));
    if (filePath.isEmpty()) {
        return;
    }
    
    // Ensure it has .tmt extension
    if (!filePath.toLower().endsWith(".tmt")) {
        filePath += ".tmt";
    }
    
    // Save the document
    document->setFilePath(filePath.toStdString());
    if (!document->save()) {
        QMessageBox::warning(this, tr("Save Error"),
                            tr("Failed to save the tape file."));
        return;
    }
    
    // Add to list
    addTapeFile(filePath);
    
    // Emit signal
    emit tapeDocumentCreated(document.release());
    emit tapeFileSelected(filePath);
}

void FileListManager::openCodeFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Code File"),
                                                 QString(), tr("Turing Machine Files (*.tm)"));
    if (filePath.isEmpty()) {
        return;
    }
    
    // Add to list and select
    if (addCodeFile(filePath)) {
        emit codeFileSelected(filePath);
    }
}

void FileListManager::openTapeFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Tape File"),
                                                 QString(), tr("Tape Files (*.tmt)"));
    if (filePath.isEmpty()) {
        return;
    }
    
    // Add to list and select
    if (addTapeFile(filePath)) {
        emit tapeFileSelected(filePath);
    }
}

void FileListManager::removeSelectedFile(bool isCodeFile)
{
    QListWidget* listWidget = isCodeFile ? m_codeFilesList : m_tapeFilesList;
    QMap<QString, QListWidgetItem*>& itemMap = isCodeFile ? m_codeItems : m_tapeItems;
    
    QListWidgetItem* item = listWidget->currentItem();
    if (!item) {
        return;
    }
    
    QString filePath = item->data(Qt::UserRole).toString();
    
    // Confirm removal
    QMessageBox::StandardButton result = QMessageBox::question(this,
        tr("Remove File"),
        tr("Are you sure you want to remove %1 from the list?").arg(filePath),
        QMessageBox::Yes | QMessageBox::No);
    
    if (result != QMessageBox::Yes) {
        return;
    }
    
    // Remove from list
    itemMap.remove(filePath);
    delete item;
    
    // Disable remove button if no selection
    if (isCodeFile) {
        m_removeCodeButton->setEnabled(m_codeFilesList->currentItem() != nullptr);
    } else {
        m_removeTapeButton->setEnabled(m_tapeFilesList->currentItem() != nullptr);
    }
}

void FileListManager::onCodeFileSelected(QListWidgetItem* item)
{
    if (!item) {
        m_removeCodeButton->setEnabled(false);
        return;
    }
    
    m_removeCodeButton->setEnabled(true);
    QString filePath = item->data(Qt::UserRole).toString();
    emit codeFileSelected(filePath);
}

void FileListManager::onTapeFileSelected(QListWidgetItem* item)
{
    if (!item) {
        m_removeTapeButton->setEnabled(false);
        return;
    }
    
    m_removeTapeButton->setEnabled(true);
    QString filePath = item->data(Qt::UserRole).toString();
    emit tapeFileSelected(filePath);
}

void FileListManager::onCodeFileDoubleClicked(QListWidgetItem* item)
{
    if (!item) {
        return;
    }
    
    QString filePath = item->data(Qt::UserRole).toString();
    emit codeFileSelected(filePath);
}

void FileListManager::onTapeFileDoubleClicked(QListWidgetItem* item)
{
    if (!item) {
        return;
    }
    
    QString filePath = item->data(Qt::UserRole).toString();
    emit tapeFileSelected(filePath);
}

void FileListManager::dragEnterEvent(QDragEnterEvent* event)
{
    // Accept drag events with URLs
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void FileListManager::dropEvent(QDropEvent* event)
{
    if (!event->mimeData()->hasUrls()) {
        return;
    }
    
    QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl& url : urls) {
        if (!url.isLocalFile()) {
            continue;
        }
        
        QString filePath = url.toLocalFile();
        QFileInfo fileInfo(filePath);
        
        if (fileInfo.suffix().toLower() == "tm") {
            // Code file
            if (addCodeFile(filePath)) {
                emit codeFileSelected(filePath);
            }
        } else if (fileInfo.suffix().toLower() == "tmt") {
            // Tape file
            if (addTapeFile(filePath)) {
                emit tapeFileSelected(filePath);
            }
        }
    }
    
    event->acceptProposedAction();
}