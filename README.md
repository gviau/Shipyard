# Shipyard

Shipyard is a PBR rendering engine, made as a learning project. Quite imcomplete for now, I'm still in the process of implementing core functionality.

# Dependencies
You need the DirectX June 2010 SDK installed, with the DXSDK_DIR environment path properly setup to point to the SDK's root path.

# How to build
1. Run ./generate_projects.bat. This will run Sharpmake and generate all the project files under ./tmp/generated-projects/
2. Open ./tmp/generated-projects/shipyard.viewer project file to compile the Shipyard-Viewer.

# Acknowledgment
This software uses the Assimp open source library. See http://www.assimp.org/ for details.
			Assimp is used under the terms of a 3-clause BSD license.

This software uses the FreeImage open source image library. See http://freeimage.sourceforge.net for details.
			FreeImage is used under the GPL, version 3.

This software uses the OpenGL Mathematics (GLM) open source library. See https://glm.g-truc.net/0.9.9/index.html for details.
			GLM is used under the Happy Bunny License (Modified MIT License).

This software uses the Dear ImGui open source library. See https://github.com/ocornut/imgui for details.
			ImGui is used under the MIT License.
			
This software uses the Sharpmake open source library. See https://github.com/ubisoftinc/Sharpmake for details.
			Sharpmake is used under the Apache License 2.0.
            
This software uses the ImGuizmo open source library. See https://github.com/CedricGuillemet/ImGuizmo for details.
			ImGuizmo is used under the MIT License.