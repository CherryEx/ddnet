/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_SHARED_DATAFILE_H
#define ENGINE_SHARED_DATAFILE_H

#include <engine/storage.h>

#include <base/hash.h>
#include <base/types.h>

#include "uuid_manager.h"

#include <cstdint>
#include <map>
#include <vector>

enum
{
	ITEMTYPE_EX = 0xFFFF,
};

// raw datafile access
class CDataFileReader
{
	class CDatafile *m_pDataFile = nullptr;

	int GetExternalItemType(int InternalType, CUuid *pUuid);
	int GetInternalItemType(int ExternalType);

public:
	~CDataFileReader();
	CDataFileReader &operator=(CDataFileReader &&Other);

	[[nodiscard]] bool Open(class IStorage *pStorage, const char *pFilename, int StorageType);
	void Close();
	bool IsOpen() const;
	IOHANDLE File() const;

	int GetDataSize(int Index) const;
	void *GetData(int Index);
	void *GetDataSwapped(int Index); // makes sure that the data is 32bit LE ints when saved
	const char *GetDataString(int Index);
	void ReplaceData(int Index, char *pData, size_t Size); // memory for data must have been allocated with malloc
	void UnloadData(int Index);
	int NumData() const;

	int GetItemSize(int Index) const;
	void *GetItem(int Index, int *pType = nullptr, int *pId = nullptr, CUuid *pUuid = nullptr);
	void GetType(int Type, int *pStart, int *pNum);
	int FindItemIndex(int Type, int Id);
	void *FindItem(int Type, int Id);
	int NumItems() const;

	SHA256_DIGEST Sha256() const;
	unsigned Crc() const;
	int MapSize() const;
};

// write access
class CDataFileWriter
{
public:
	enum ECompressionLevel
	{
		COMPRESSION_DEFAULT,
		COMPRESSION_BEST,
	};

private:
	class CDataInfo
	{
	public:
		void *m_pUncompressedData;
		int m_UncompressedSize;
		void *m_pCompressedData;
		int m_CompressedSize;
		ECompressionLevel m_CompressionLevel;
	};

	class CItemInfo
	{
	public:
		int m_Type;
		int m_Id;
		int m_Size;
		int m_Next;
		int m_Prev;
		void *m_pData;
	};

	class CItemTypeInfo
	{
	public:
		int m_Num = 0;
		int m_First = -1;
		int m_Last = -1;
	};

	class CExtendedItemType
	{
	public:
		int m_Type;
		CUuid m_Uuid;
	};

	IOHANDLE m_File;
	std::map<uint16_t, CItemTypeInfo, std::less<>> m_ItemTypes; // item types must be sorted in ascending order
	std::vector<CItemInfo> m_vItems;
	std::vector<CDataInfo> m_vDatas;
	std::vector<CExtendedItemType> m_vExtendedItemTypes;

	int GetTypeFromIndex(int Index) const;
	int GetExtendedItemTypeIndex(int Type, const CUuid *pUuid);

public:
	CDataFileWriter();
	CDataFileWriter(CDataFileWriter &&Other)
	{
		m_File = Other.m_File;
		Other.m_File = nullptr;
		m_ItemTypes = std::move(Other.m_ItemTypes);
		m_vItems = std::move(Other.m_vItems);
		m_vDatas = std::move(Other.m_vDatas);
		m_vExtendedItemTypes = std::move(Other.m_vExtendedItemTypes);
	}
	~CDataFileWriter();

	[[nodiscard]] bool Open(class IStorage *pStorage, const char *pFilename, int StorageType = IStorage::TYPE_SAVE);
	int AddItem(int Type, int Id, size_t Size, const void *pData, const CUuid *pUuid = nullptr);
	int AddData(size_t Size, const void *pData, ECompressionLevel CompressionLevel = COMPRESSION_DEFAULT);
	int AddDataSwapped(size_t Size, const void *pData);
	int AddDataString(const char *pStr);
	void Finish();
};

#endif
