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
#ifndef __ANDROID_ASSET_LOADER__
#define __ANDROID_ASSET_LOADER__

#ifndef __ANDROID__
#error this header file has to be included on android port only!
#endif

#include <android/asset_manager.h>
#include <IFileArchive.h>
#include "irrlichttypes.h"

class AssetReadFile : public io::ReadFile {
public:

	AssetReadFile(AAsset *asset, core::string<fschar_t> fname):
		m_asset(asset), m_filename(fname)
	{}
	~AssetReadFile()
	{
		AAsset_close(m_asset);
	}
	virtual s32 read(void* buffer, u32 sizeToRead)
	{
		return AAsset_read(m_asset, buffer, sizeToRead);
	}

	virtual bool seek(long finalPos, bool relativeMovement = false)
	{
		// TODO
	}

	virtual long getSize() const
	{
		AAsset_getLength64(m_asset);
	}

	virtual long getPos() const
	{
		// TODO
	}

	virtual const core::string<fschar_t> &getFileName() const
	{
		return m_filename;
	}
private:
	AAsset *m_asset;
	core::string<fschar_t> m_filename;
};

class AssetManagerArchive : public io::IFileArchive {
	AssetManagerLoader(AAssetManager *mgr): m_mgr(mgr)
	{}

	virtual io::IReadFile *createAndOpenFile(const core::string<fschar_t> &filename)
	{
		if (filename.find("asset://") != 0)
			return NULL;
		core::string<fschar_t> trimmed = filename.subString(8, filename.size() - 8);
		AAsset *asset = AAssetManager_open(m_mgr, trimmed, AASSET_MODE_UNKNOWN);
		return new AssetReadFile(asset, filename);
	}
	virtual io::IReadFile *createAndOpenFile(u32 index)
	{
		return NULL;
	}
	virtual const io::IFileList *getFileList()
	{
		return NULL;
	}
	virtual io::E_FILE_ARCHIVE_TYPE getType()
	{
		return io::EFAT_UNKNOWN;
	}
private:
	AAssetManager *m_mgr;
};

class AssetManagerLoader : public io::IArchiveLoader {
public:
	AssetManagerLoader(AAssetManager *mgr): m_mgr(mgr)
	{}

	virtual io::IFileArchive *createArchive(const core::string<fschar_t> &filename, bool ignoreCase, bool ignorePaths) const
	{
		if (filename == "asset_fake_archive") {
			return AssetManagerLoader(m_mgr);
		} else {
			return NULL;
		}
	}
	virtual io::IFileArchive *createArchive(io::IReadFile *file, bool ignoreCase, bool ignorePaths) const
	{
		return NULL;
	}
	virtual bool isALoadableFileFormat(const core::string<fschar_t> &filename) const
	{
		return filename == "asset_fake_archive";
	}
	virtual bool isALoadableFileFormat(io::IReadFile *file) const
	{
		return false;
	}
	virtual bool isALoadableFileFormat(io::E_FILE_ARCHIVE_TYPE fileType) const
	{
		return false;
	}
private:
	AAssetManager *m_mgr;
};
#endif
