#ifndef BATCHLOAD_H
#define BATCHLOAD_H

#include <FreeImage.h>

FIBITMAP* GenericLoader(const char* lpszPathName, int flag);

bool GenericWriter(FIBITMAP* dib, const char* lpszPathName, int flag);

#endif
