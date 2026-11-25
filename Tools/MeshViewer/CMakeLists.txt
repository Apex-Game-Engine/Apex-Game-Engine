cmake_minimum_required(VERSION 3.19)

project(MeshViewer)

add_apex_tool(ApexMeshViewer
	TITLE "Apex Mesh Viewer"
	VERSION "0.0.1"
	USE_GUI
	USE_CONSOLE
	SOURCES
		src/AssetStore.h
		src/Main.cpp
		MainScript.lua
	LINKS
		ApexAsset
		ApexMount
		lua5.3
	DEPENDS
		ApexShaders
)