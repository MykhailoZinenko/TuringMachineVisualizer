#include "TuringMachineSyntaxHighlighter.h"

TuringMachineSyntaxHighlighter::TuringMachineSyntaxHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    // Set up formats
    
    // Function notation format (f, ->)
    m_functionFormat.setForeground(Qt::darkBlue);
    m_functionFormat.setFontWeight(QFont::Bold);
    
    // State format (q0, q1, etc.)
    m_stateFormat.setForeground(Qt::darkGreen);
    
    // Symbol format (0, 1, Blank, etc.)
    m_symbolFormat.setForeground(Qt::red);
    
    // Direction format (L, R, N)
    m_directionFormat.setForeground(Qt::darkMagenta);
    
    // Comment format
    m_commentFormat.setForeground(Qt::gray);
    
    // Punctuation format
    m_punctuationFormat.setForeground(Qt::black);
    m_punctuationFormat.setFontWeight(QFont::Bold);
    
    // State declaration format (s, a, r, q)
    m_stateDeclFormat.setForeground(Qt::blue);
    m_stateDeclFormat.setFontWeight(QFont::Bold);
    
    // Function notation rule - matches f(q0, 0)
    HighlightingRule rule;
    rule.pattern = QRegularExpression(R"(f\s*\()");
    rule.format = m_functionFormat;
    m_highlightingRules.append(rule);
    
    // Arrow rule - matches -> or =
    rule.pattern = QRegularExpression(R"(->|=)");
    rule.format = m_functionFormat;
    m_highlightingRules.append(rule);
    
    // State rule - matches q0, q1, etc.
    rule.pattern = QRegularExpression(R"(\b[qsa][0-9a-zA-Z_]+\b)");
    rule.format = m_stateFormat;
    m_highlightingRules.append(rule);
    
    // Symbol rule - matches 0, 1, "Blank", _, etc.
    rule.pattern = QRegularExpression(R"(\b(?:Blank|blank|_|[0-9a-zA-Z])\b)");
    rule.format = m_symbolFormat;
    m_highlightingRules.append(rule);
    
    // Direction rule - matches L, R, N, S, 0
    rule.pattern = QRegularExpression(R"(\b(?:L|R|N|S|0)\b)");
    rule.format = m_directionFormat;
    m_highlightingRules.append(rule);
    
    // Comment rule - matches // to end of line
    rule.pattern = QRegularExpression(R"(//[^\n]*)");
    rule.format = m_commentFormat;
    m_highlightingRules.append(rule);
    
    // Punctuation rule - matches (, ), ,
    rule.pattern = QRegularExpression(R"([\(\),])");
    rule.format = m_punctuationFormat;
    m_highlightingRules.append(rule);
    
    // State declaration rule - matches s, a, r, q as standalone letters followed by (
    rule.pattern = QRegularExpression(R"(\b(s|a|r|q)\s*\()");
    rule.format = m_stateDeclFormat;
    m_highlightingRules.append(rule);
}

void TuringMachineSyntaxHighlighter::highlightBlock(const QString& text)
{
    // Apply each rule to the text
    for (const HighlightingRule& rule : m_highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}