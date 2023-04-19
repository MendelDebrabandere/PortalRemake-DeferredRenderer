#include "stdafx.h"
#include "SpriteFontLoader.h"

SpriteFont* SpriteFontLoader::LoadContent(const ContentLoadInfo& loadInfo)
{
	const auto pReader = new BinaryReader();
	pReader->Open(loadInfo.assetFullPath);

	if (!pReader->Exists())
	{
		Logger::LogError(L"Failed to read the assetFile!\nPath: \'{}\'", loadInfo.assetSubPath);
		return nullptr;
	}

	//TODO_W7(L"Implement SpriteFontLoader >> Parse .fnt file");
	//See BMFont Documentation for Binary Layout

	//Parse the Identification bytes (B,M,F)
	//If Identification bytes doesn't match B|M|F,
	//Log Error (SpriteFontLoader::LoadContent > Not a valid .fnt font) &
	//return nullptr
	const char identifier0{ pReader->Read<char>() };
	const char identifier1{ pReader->Read<char>() };
	const char identifier2{ pReader->Read<char>() };
	if (identifier0 != 'B' && identifier1 != 'M' && identifier2 != 'F')
	{
		Logger::LogError(L"SpriteFontLoader::LoadContent > Not a valid .fnt font");
		return nullptr;
	}

	//Parse the version (version 3 required)
	//If version is < 3,
	//Log Error (SpriteFontLoader::LoadContent > Only .fnt version 3 is supported)
	//return nullptr
	if (pReader->Read<char>() != 3)
	{
		Logger::LogError(L"SpriteFontLoader::LoadContent > Only .fnt version 3 is supported");
		return nullptr;
	}

	//Valid .fnt file >> Start Parsing!
	//use this SpriteFontDesc to store all relevant information (used to initialize a SpriteFont object)
	SpriteFontDesc fontDesc{};

	//**********
	// BLOCK 0 *
	//**********
	//Retrieve the blockId and blockSize
	//Retrieve the FontSize [fontDesc.fontSize]
	//Move the binreader to the start of the FontName [BinaryReader::MoveBufferPosition(...) or you can set its position using BinaryReader::SetBufferPosition(...))
	//Retrieve the FontName [fontDesc.fontName]
	pReader->Read<char>();
	pReader->Read<int>();
	fontDesc.fontSize = pReader->Read<int16_t>();
	pReader->MoveBufferPosition(12);
	fontDesc.fontName = pReader->ReadNullString();

	//**********
	// BLOCK 1 *
	//**********
	//Retrieve the blockId and blockSize
	//Retrieve Texture Width & Height [fontDesc.textureWidth/textureHeight]
	//Retrieve PageCount
	//> if pagecount > 1
	//	> Log Error (Only one texture per font is allowed!)
	//Advance to Block2 (Move Reader)
	pReader->Read<char>();
	pReader->Read<int>();
	pReader->MoveBufferPosition(4);
	fontDesc.textureWidth = pReader->Read<uint16_t>();
	fontDesc.textureHeight = pReader->Read<uint16_t>();
	const int pages = pReader->Read<uint16_t>();
	if (pages > 1)
	{
		Logger::LogError(L"Only one texture per font is allowed!");
		return nullptr;
	}
	pReader->MoveBufferPosition(5);


	//**********
	// BLOCK 2 *
	//**********
	//Retrieve the blockId and blockSize
	//Retrieve the PageName (BinaryReader::ReadNullString)
	//Construct the full path to the page texture file
	//	>> page texture should be stored next to the .fnt file, pageName contains the name of the texture file
	//	>> full texture path = asset parent_path of .fnt file (see loadInfo.assetFullPath > get parent_path) + pageName (filesystem::path::append)
	//	>> Load the texture (ContentManager::Load<TextureData>) & Store [fontDesc.pTexture]
	pReader->Read<char>();
	pReader->Read<int>();
	const std::wstring pageName{ pReader->ReadNullString() };
	fontDesc.pTexture = ContentManager::Load<TextureData>(loadInfo.assetFullPath.parent_path().append(pageName).wstring());

	//**********
	// BLOCK 3 *
	//**********
	//Retrieve the blockId and blockSize
	//Retrieve Character Count (see documentation)
	//Create loop for Character Count, and:
	//> Retrieve CharacterId (store Local) and cast to a 'wchar_t'
	//> Create instance of FontMetric (struct)
	//	> Set Character (CharacterId) [FontMetric::character]
	//	> Retrieve Xposition (store Local)
	//	> Retrieve Yposition (store Local)
	//	> Retrieve & Set Width [FontMetric::width]
	//	> Retrieve & Set Height [FontMetric::height]
	//	> Retrieve & Set OffsetX [FontMetric::offsetX]
	//	> Retrieve & Set OffsetY [FontMetric::offsetY]
	//	> Retrieve & Set AdvanceX [FontMetric::advanceX]
	//	> Retrieve & Set Page [FontMetric::page]
	//	> Retrieve Channel (BITFIELD!!!) 
	//		> See documentation for BitField meaning [FontMetrix::channel]
	//	> Calculate Texture Coordinates using Xposition, Yposition, fontDesc.TextureWidth & fontDesc.TextureHeight [FontMetric::texCoord]
	//
	//> Insert new FontMetric to the metrics [font.metrics] map
	//	> key = (wchar_t) charId
	//	> value = new FontMetric
	//(loop restarts till all metrics are parsed)
	pReader->Read<char>();
	int blockSize = pReader->Read<int>();
	const int numChars{ blockSize / 20 };
	for (int i{}; i < numChars; ++i)
	{
		const unsigned int charId{ pReader->Read<unsigned int>() };
		FontMetric character{};
		character.character = static_cast<wchar_t>(charId);
		const uint16_t xPos{ pReader->Read<uint16_t>() };
		const uint16_t yPos{ pReader->Read<uint16_t>() };
		character.width = pReader->Read<uint16_t>();
		character.height = pReader->Read<uint16_t>();
		character.offsetX = pReader->Read<uint16_t>();
		character.offsetY = pReader->Read<uint16_t>();
		character.advanceX = pReader->Read<uint16_t>();
		character.page = pReader->Read<unsigned char>();

		const unsigned char channelBitField{ pReader->Read<unsigned char>() };
		switch (channelBitField)
		{
		case 0x1:
			character.channel = 2;
			break;
		case 0x2:
			character.channel = 1;
			break;
		case 0x4:
			character.channel = 0;
			break;
		case 0x8:
			character.channel = 4;
			break;
		case 0xF:
		default:
			character.channel = 0;
		}

		character.texCoord = XMFLOAT2
		{
			static_cast<float>(xPos) / fontDesc.textureWidth,
			static_cast<float>(yPos) / fontDesc.textureWidth
		};
		fontDesc.metrics[character.character] = character;
	}

	//Done!
	delete pReader;
	return new SpriteFont(fontDesc);
}

void SpriteFontLoader::Destroy(SpriteFont* objToDestroy)
{
	SafeDelete(objToDestroy);
}
