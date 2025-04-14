#pragma once

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QVector>
#include <QTextCharFormat>

/**
 * Syntax highlighter for Turing machine code
 */
class TuringMachineSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @param parent The parent document
     */
    explicit TuringMachineSyntaxHighlighter(QTextDocument* parent);

protected:
    /**
     * Highlight the given text block
     * @param text The text to highlight
     */
    void highlightBlock(const QString& text) override;

private:
    /**
     * Structure for highlighting rules
     */
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    // Vector of highlighting rules
    QVector<HighlightingRule> m_highlightingRules;
    
    // Formats for different syntax elements
    QTextCharFormat m_functionFormat;
    QTextCharFormat m_stateFormat;
    QTextCharFormat m_symbolFormat;
    QTextCharFormat m_directionFormat;
    QTextCharFormat m_commentFormat;
    QTextCharFormat m_punctuationFormat;
    QTextCharFormat m_stateDeclFormat;
};