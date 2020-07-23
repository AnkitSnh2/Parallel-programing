// ==========================================================
// Batch loader
//
// Design and implementation by
// - Floris van den Berg
// - Herv� Drolon
//
// This file is part of FreeImage 3
//
// COVERED CODE IS PROVIDED UNDER THIS LICENSE ON AN "AS IS" BASIS, WITHOUT WARRANTY
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, WITHOUT LIMITATION, WARRANTIES
// THAT THE COVERED CODE IS FREE OF DEFECTS, MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE
// OR NON-INFRINGING. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE COVERED
// CODE IS WITH YOU. SHOULD ANY COVERED CODE PROVE DEFECTIVE IN ANY RESPECT, YOU (NOT
// THE INITIAL DEVELOPER OR ANY OTHER CONTRIBUTOR) ASSUME THE COST OF ANY NECESSARY
// SERVICING, REPAIR OR CORRECTION. THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL
// PART OF THIS LICENSE. NO USE OF ANY COVERED CODE IS AUTHORIZED HEREUNDER EXCEPT UNDER
// THIS DISCLAIMER.
//
// Use at own risk!
// ==========================================================

//
//  This example shows how to easily batch load a directory
//  full of images. Because not all formats can be identified
//  by their header (some images don't have a header or one
//  at the end of the file) we make use of the
//  FreeImage_GetFIFFromFilename function. This function
//  receives a file name, for example 'myfile.bmp', and returns
//  a FREE_IMAGE_TYPE enum which identifies that bitmap.
//
//  Functions used in this sample :
//  FreeImage_GetFileType, FreeImage_GetFIFFromFilename, FreeImage_FIFSupportsReading,
//  FreeImage_Load, FreeImage_GetBPP, FreeImage_FIFSupportsWriting, FreeImage_GetFormatFromFIF
//  FreeImage_FIFSupportsExportBPP, FreeImage_Save, FreeImage_Unload,
//  FreeImage_SetOutputMessage, FreeImage_GetVersion, FreeImage_GetCopyrightMessage
//
// ==========================================================

#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>

#include <FreeImage.h>

// ----------------------------------------------------------

/** Generic image loader
	@param lpszPathName Pointer to the full file name
	@param flag Optional load flag constant
	@return Returns the loaded dib if successful, returns NULL otherwise
*/
FIBITMAP* GenericLoader(const char* lpszPathName, int flag) {
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

	// check the file signature and deduce its format
	// (the second argument is currently not used by FreeImage)
	fif = FreeImage_GetFileType(lpszPathName, 0);
	if(fif == FIF_UNKNOWN) {
		// no signature ?
		// try to guess the file format from the file extension
		fif = FreeImage_GetFIFFromFilename(lpszPathName);
	}
	// check that the plugin has reading capabilities ...
	if((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
		// ok, let's load the file
		FIBITMAP *dib = FreeImage_Load(fif, lpszPathName, flag);
		// unless a bad file format, we are done !
		return dib;
	}
	return NULL;
}

/** Generic image writer
	@param dib Pointer to the dib to be saved
	@param lpszPathName Pointer to the full file name
	@param flag Optional save flag constant
	@return Returns true if successful, returns false otherwise
*/
bool GenericWriter(FIBITMAP* dib, const char* lpszPathName, int flag) {
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	BOOL bSuccess = FALSE;

	if(dib) {
		// try to guess the file format from the file extension
		fif = FreeImage_GetFIFFromFilename(lpszPathName);
		if(fif != FIF_UNKNOWN ) {
			// check that the plugin has sufficient writing and export capabilities ...
			WORD bpp = FreeImage_GetBPP(dib);
			if(FreeImage_FIFSupportsWriting(fif) && FreeImage_FIFSupportsExportBPP(fif, bpp)) {
				// ok, we can save the file
				bSuccess = FreeImage_Save(fif, dib, lpszPathName, flag);
				std::cout<<"File saved at "<< lpszPathName<<std::endl;
				// unless an abnormal bug, we are done !
			}
		}
	}
	return (bSuccess == TRUE) ? true : false;
}

// ----------------------------------------------------------

/**
	FreeImage error handler
	@param fif Format / Plugin responsible for the error
	@param message Error message
*/
// void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message) {
// 	printf("\n*** ");
// 	if(fif != FIF_UNKNOWN) {
// 		printf("%s Format\n", FreeImage_GetFormatFromFIF(fif));
// 	}
// 	printf(message);
// 	printf(" ***\n");
// }
