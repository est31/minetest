/*
Minetest
Copyright (C) 2015 est31, <MTest31@outlook.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "porting.h"
#include "android_asset_funopen.h"

#include <stdio.h>
#include <android/asset_manager.h>
#include <errno.h>

// Bases on http://www.50ply.com/blog/2013/01/19/loading-compressed-android-assets-with-file-pointer/

static int android_read(void *handle, char *buf, int size)
{
	return AAsset_read((AAsset *)handle, buf, size);
}

static int android_write(void *handle, const char *buf, int size)
{
	return EACCES; // no writes possible
}

static fpos_t android_seek(void *handle, fpos_t offset, int whence)
{
	return AAsset_seek((AAsset *)handle, offset, whence);
}

static int android_close(void *handle)
{
	AAsset_close((AAsset *)handle);
	return 0;
}

FILE* android_fopen(const char *fname, const char *mode) {
	if (strncmp("asset://", fname, 8) != 0) {
		return fopen(fname, mode);
	}
	if (strstr(mode, "w") != NULL) return NULL;

	AAsset *asset = AAssetManager_open(porting::g_asset_manager, fname + 8, 0);
	if (!asset) return NULL;

	return funopen(asset, android_read, android_write, android_seek, android_close);
}
