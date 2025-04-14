#pragma once

#include <QString>
#include <QColor>
#include <QFont>
#include <QApplication>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>

/*
 * StyleKit: Application-wide style system for the Turing Machine Visualizer
 * Inspired by modern IDE aesthetics (VS Code, JetBrains)
 */
class StyleKit {
public:
    // Theme modes
    enum ThemeMode {
        LIGHT,
        DARK
    };

    // Apply the style to the entire application
    static void applyToApplication(ThemeMode mode = LIGHT) {
        QApplication* app = qobject_cast<QApplication*>(QApplication::instance());
        if (!app) return;

        if (mode == DARK) {
            applyDarkTheme(app);
        } else {
            applyLightTheme(app);
        }

        // Apply common styles
        applyCommonStyles(app);
    }

    // Get style sheet for specific widget types
    static QString getButtonStyle(bool primary = true) {
        if (primary) {
            return QString(
                "QPushButton {"
                "  background-color: #007ACC;"  // VS Code blue
                "  color: white;"
                "  border: none;"
                "  border-radius: 3px;"
                "  padding: 6px 12px;"
                "  font-weight: 500;"
                "}"
                "QPushButton:hover {"
                "  background-color: #0066B8;"  // Darker blue
                "}"
                "QPushButton:pressed {"
                "  background-color: #005BA1;"  // Even darker blue
                "}"
                "QPushButton:disabled {"
                "  background-color: #CCCCCC;"  // Light gray
                "  color: #9A9A9A;"             // Medium gray
                "}"
            );
        } else {
            return QString(
                "QPushButton {"
                "  background-color: #ECECEC;"  // Light gray
                "  color: #333333;"             // Dark text
                "  border: 1px solid #BDBDBD;"  // Light border
                "  border-radius: 3px;"
                "  padding: 5px 11px;"
                "  font-weight: 500;"
                "}"
                "QPushButton:hover {"
                "  background-color: #E0E0E0;"  // Slightly darker gray
                "}"
                "QPushButton:pressed {"
                "  background-color: #D0D0D0;"  // Even darker gray
                "}"
                "QPushButton:disabled {"
                "  background-color: #F5F5F5;"  // Nearly white
                "  color: #BDBDBD;"             // Light gray
                "  border: 1px solid #E0E0E0;"  // Light gray
                "}"
            );
        }
    }

    static QString getInputStyle() {
        return QString(
            "QLineEdit, QTextEdit, QSpinBox {"
            "  background-color: #FFFFFF;"
            "  border: 1px solid #BDBDBD;"  // Light gray border
            "  border-radius: 3px;"
            "  padding: 6px;"
            "  color: #333333;"             // Dark text
            "}"
            "QLineEdit:focus, QTextEdit:focus, QSpinBox:focus {"
            "  border: 1px solid #007ACC;"  // Blue border when focused
            "  outline: none;"
            "}"
            "QLineEdit:disabled, QTextEdit:disabled, QSpinBox:disabled {"
            "  background-color: #F5F5F5;"  // Very light gray
            "  color: #9A9A9A;"             // Medium gray
            "}"
            // Style for the up/down buttons in QSpinBox
            "QSpinBox::up-button, QSpinBox::down-button {"
            "  border: none;"
            "  background-color: #E0E0E0;"  // Light gray
            "  width: 14px;"
            "}"
            "QSpinBox::up-button:hover, QSpinBox::down-button:hover {"
            "  background-color: #D0D0D0;"  // Slightly darker gray
            "}"
        );
    }

    static QString getTabStyle() {
        return QString(
            "QTabWidget::pane {"
            "  border: 1px solid #BDBDBD;"  // Light gray border
            "  border-radius: 3px;"
            "  top: -1px;"
            "}"
            "QTabBar::tab {"
            "  background-color: #ECECEC;"  // Very light gray
            "  border: 1px solid #BDBDBD;"  // Light gray border
            "  border-bottom: none;"
            "  border-top-left-radius: 3px;"
            "  border-top-right-radius: 3px;"
            "  padding: 6px 12px;"
            "  margin-right: 2px;"
            "  color: #6C6C6C;"             // Medium gray text
            "}"
            "QTabBar::tab:selected {"
            "  background-color: #FFFFFF;"  // White
            "  border-bottom: none;"
            "  color: #333333;"             // Dark text
            "  font-weight: 500;"
            "}"
            "QTabBar::tab:hover:!selected {"
            "  background-color: #E0E0E0;"  // Slightly darker light gray
            "}"
        );
    }

    static QString getGroupBoxStyle() {
        return QString(
            "QGroupBox {"
            "  border: 1px solid #BDBDBD;"  // Light gray border
            "  border-radius: 3px;"
            "  margin-top: 20px;"          // Increased margin-top for title space
            "  font-weight: 500;"
            "  color: #333333;"             // Dark text
            "  background-color: white;"
            "}"
            "QGroupBox::title {"
            "  subcontrol-origin: margin;"
            "  subcontrol-position: top left;"
            "  left: 10px;"
            "  top: 10px;"                // Adjusted top position to prevent cutoff
            "  padding: 0 5px;"
            "  background-color: white;"   // Match background to ensure visibility
            "}"
        );
    }

    static QString getDockStyle() {
        return QString(
            "QDockWidget {"
            "  titlebar-close-icon: url(:/icons/close.svg);"
            "  titlebar-normal-icon: url(:/icons/maximize.svg);"
            "}"
            "QDockWidget::title {"
            "  background-color: #F5F5F5;"  // Very light gray
            "  padding-left: 10px;"
            "  padding-top: 4px;"
            "  padding-bottom: 4px;"
            "  font-weight: 500;"
            "  color: #333333;"             // Dark text
            "}"
            "QDockWidget::close-button, QDockWidget::float-button {"
            "  background-color: transparent;"
            "  icon-size: 14px;"
            "  padding: 2px;"
            "}"
            "QDockWidget::close-button:hover, QDockWidget::float-button:hover {"
            "  background-color: #E0E0E0;"  // Light gray
            "  border-radius: 2px;"
            "}"
        );
    }

    static QString getTapeWidgetStyle() {
        return QString(
            "TapeWidget {"
            "  background-color: #FAFAFA;"  // Very light background
            "  border: 1px solid #BDBDBD;"  // Light gray border
            "  border-radius: 3px;"
            "}"
        );
    }

    static QString getSliderStyle() {
        return QString(
            "QSlider::groove:horizontal {"
            "  border: 1px solid #BDBDBD;"  // Light gray border
            "  height: 6px;"
            "  background: #E0E0E0;"  // Light gray
            "  border-radius: 3px;"
            "}"
            "QSlider::handle:horizontal {"
            "  background: #007ACC;"  // VS Code blue
            "  border: none;"
            "  width: 16px;"
            "  margin: -5px 0;"
            "  border-radius: 8px;"
            "}"
            "QSlider::handle:horizontal:hover {"
            "  background: #0066B8;"  // Darker blue
            "}"
        );
    }

    static QString getCodeEditorStyle() {
        return QString(
            "QTextEdit {"
            "  background-color: #FFFFFF;"  // White background
            "  border: none;"
            "  color: #333333;"             // Dark text
            "  selection-background-color: #ADD6FF;"  // Light blue selection
            "  selection-color: #333333;"   // Dark text in selection
            "  font-family: 'JetBrains Mono', 'Consolas', 'Courier New', monospace;"
            "  font-size: 11pt;"
            "  line-height: 1.4;"
            "}"
            // Add line numbers styling here if available
        );
    }

    static QString getToolbarStyle() {
        return QString(
            "QToolBar {"
            "  background-color: #F5F5F5;"  // Very light gray
            "  border-bottom: 1px solid #BDBDBD;"  // Light border
            "  spacing: 2px;"
            "  padding: 2px;"
            "}"
            "QToolButton {"
            "  background-color: transparent;"
            "  border: none;"
            "  border-radius: 3px;"
            "  padding: 4px;"
            "}"
            "QToolButton:hover {"
            "  background-color: #E0E0E0;"  // Light gray on hover
            "}"
            "QToolButton:pressed {"
            "  background-color: #D0D0D0;"  // Slightly darker gray
            "}"
            "QToolButton:checked {"
            "  background-color: #CCCCCC;"  // Medium gray when checked
            "}"
        );
    }

    static QString getSearchWidgetStyle() {
        return QString(
            "QWidget#searchWidget {"
            "  background-color: #F5F5F5;"  // Very light gray
            "  border: 1px solid #BDBDBD;"  // Light gray border
            "  border-radius: 3px;"
            "}"
            "QLineEdit#searchField {"
            "  border: 1px solid #BDBDBD;"  // Light gray border
            "  border-radius: 2px;"
            "  padding: 4px 6px;"
            "  background-color: white;"
            "  selection-background-color: #007ACC;"  // VS Code blue for selection
            "}"
            "QPushButton#closeSearchButton {"
            "  border: none;"
            "  background-color: transparent;"
            "  color: #666666;"
            "  padding: 2px;"
            "  font-weight: bold;"
            "}"
            "QPushButton#closeSearchButton:hover {"
            "  background-color: #E0E0E0;"  // Light gray on hover
            "  border-radius: 2px;"
            "}"
            "QCheckBox {"
            "  spacing: 4px;"
            "}"
            "QCheckBox::indicator {"
            "  width: 14px;"
            "  height: 14px;"
            "}"
        );
    }

private:
    // Apply light theme
    static void applyLightTheme(QApplication* app) {
        QPalette palette;
        palette.setColor(QPalette::Window, QColor("#FFFFFF"));
        palette.setColor(QPalette::WindowText, QColor("#333333"));
        palette.setColor(QPalette::Base, QColor("#FFFFFF"));
        palette.setColor(QPalette::AlternateBase, QColor("#F5F5F5"));
        palette.setColor(QPalette::ToolTipBase, QColor("#FFFFFF"));
        palette.setColor(QPalette::ToolTipText, QColor("#333333"));
        palette.setColor(QPalette::Text, QColor("#333333"));
        palette.setColor(QPalette::Button, QColor("#ECECEC"));
        palette.setColor(QPalette::ButtonText, QColor("#333333"));
        palette.setColor(QPalette::BrightText, QColor("#FFFFFF"));
        palette.setColor(QPalette::Link, QColor("#007ACC"));
        palette.setColor(QPalette::Highlight, QColor("#007ACC"));
        palette.setColor(QPalette::HighlightedText, QColor("#FFFFFF"));

        // Disabled colors
        palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor("#9A9A9A"));
        palette.setColor(QPalette::Disabled, QPalette::Text, QColor("#9A9A9A"));
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#9A9A9A"));

        app->setPalette(palette);
    }

    // Apply dark theme
    static void applyDarkTheme(QApplication* app) {
        QPalette palette;
        palette.setColor(QPalette::Window, QColor("#252526"));        // VS Code dark bg
        palette.setColor(QPalette::WindowText, QColor("#E0E0E0"));    // Light text
        palette.setColor(QPalette::Base, QColor("#1E1E1E"));          // Editor bg
        palette.setColor(QPalette::AlternateBase, QColor("#303030")); // Alternate
        palette.setColor(QPalette::ToolTipBase, QColor("#2D2D30"));   // Tooltip bg
        palette.setColor(QPalette::ToolTipText, QColor("#E0E0E0"));   // Tooltip text
        palette.setColor(QPalette::Text, QColor("#E0E0E0"));          // Text
        palette.setColor(QPalette::Button, QColor("#3C3C3C"));        // Button
        palette.setColor(QPalette::ButtonText, QColor("#E0E0E0"));    // Button text
        palette.setColor(QPalette::BrightText, QColor("#FFFFFF"));    // Bright text
        palette.setColor(QPalette::Link, QColor("#007ACC"));          // Link blue
        palette.setColor(QPalette::Highlight, QColor("#007ACC"));     // Highlight
        palette.setColor(QPalette::HighlightedText, QColor("#FFFFFF"));// Highlighted text

        // Disabled colors
        palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor("#6D6D6D"));
        palette.setColor(QPalette::Disabled, QPalette::Text, QColor("#6D6D6D"));
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#6D6D6D"));

        app->setPalette(palette);
    }

    // Apply common styles
    static void applyCommonStyles(QApplication* app) {
        // Set application-wide style sheet
        app->setStyleSheet(
            // Global font settings
            "* {"
            "  font-family: 'Segoe UI', 'SF Pro Text', system-ui, sans-serif;"
            "  font-size: 9pt;"
            "}"

            // QMainWindow styling
            "QMainWindow {"
            "  background-color: palette(window);"
            "}"

            // Status bar styling
            "QStatusBar {"
            "  background-color: palette(window);"
            "  border-top: 1px solid #BDBDBD;"
            "}"
            "QStatusBar::item {"
            "  border: none;"
            "}"

            // Menu styling
            "QMenuBar {"
            "  background-color: palette(window);"
            "  border-bottom: 1px solid #BDBDBD;"
            "}"
            "QMenuBar::item {"
            "  padding: 6px 10px;"
            "  background: transparent;"
            "}"
            "QMenuBar::item:selected {"
            "  background-color: #E0E0E0;"
            "  border-radius: 3px;"
            "}"
            "QMenu {"
            "  background-color: palette(window);"
            "  border: 1px solid #BDBDBD;"
            "  border-radius: 3px;"
            "  padding: 4px 0px;"
            "}"
            "QMenu::item {"
            "  padding: 4px 28px 4px 12px;"
            "  min-width: 150px;"
            "}"
            "QMenu::item:selected {"
            "  background-color: #E0E0E0;"
            "  color: palette(text);"
            "}"
            "QMenu::separator {"
            "  height: 1px;"
            "  background-color: #BDBDBD;"
            "  margin: 4px 8px;"
            "}"

            // Toolbar styling is in getToolbarStyle()

            // Scrollbar styling
            "QScrollBar:vertical {"
            "  border: none;"
            "  background-color: #F5F5F5;"
            "  width: 10px;"
            "  margin: 0px;"
            "}"
            "QScrollBar::handle:vertical {"
            "  background-color: #CDCDCD;"
            "  border-radius: 5px;"
            "  min-height: 20px;"
            "  margin: 1px;"
            "}"
            "QScrollBar::handle:vertical:hover {"
            "  background-color: #BEBEBE;"
            "}"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
            "  height: 0px;"
            "}"
            "QScrollBar:horizontal {"
            "  border: none;"
            "  background-color: #F5F5F5;"
            "  height: 10px;"
            "  margin: 0px;"
            "}"
            "QScrollBar::handle:horizontal {"
            "  background-color: #CDCDCD;"
            "  border-radius: 5px;"
            "  min-width: 20px;"
            "  margin: 1px;"
            "}"
            "QScrollBar::handle:horizontal:hover {"
            "  background-color: #BEBEBE;"
            "}"
            "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
            "  width: 0px;"
            "}"
        );
    }
};