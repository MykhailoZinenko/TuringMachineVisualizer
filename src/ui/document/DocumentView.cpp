#include "DocumentView.h"
#include "../../document/Document.h"

DocumentView::DocumentView(Document* document, QWidget* parent)
    : QWidget(parent), m_document(document)
{
}

DocumentView::~DocumentView()
{
}