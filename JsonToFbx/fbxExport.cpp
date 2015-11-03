#include "fbxsdk.h"
#include "stdio.h"
#include <cstdio>
#include <string>
#include "rapidjson/document.h"
#include <vector>


using namespace rapidjson;
using namespace std;

FbxManager* pManager;
FbxScene* pScene;
vector<FbxNode*> fbxBones;


#define IOSETTING_REF (*(pManager->GetIOSettings()))

bool SaveScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename, int pFileFormat = -1, bool pEmbedMedia = false)
{
	int lMajor, lMinor, lRevision;
	bool lStatus = true;

	// Create an exporter.
	FbxExporter* lExporter = FbxExporter::Create(pManager, "");

	if (pFileFormat < 0 || pFileFormat >= pManager->GetIOPluginRegistry()->GetWriterFormatCount())
	{
		// Write in fall back format in less no ASCII format found
		pFileFormat = pManager->GetIOPluginRegistry()->GetNativeWriterFormat();

		//Try to export in ASCII if possible
		int lFormatIndex, lFormatCount = pManager->GetIOPluginRegistry()->GetWriterFormatCount();

		for (lFormatIndex = 0; lFormatIndex < lFormatCount; lFormatIndex++)
		{
			if (pManager->GetIOPluginRegistry()->WriterIsFBX(lFormatIndex))
			{
				FbxString lDesc = pManager->GetIOPluginRegistry()->GetWriterFormatDescription(lFormatIndex);
				const char *lASCII = "ascii";
				if (lDesc.Find(lASCII) >= 0)
				{
					pFileFormat = lFormatIndex;
					break;
				}
			}
		}
	}

	// Set the export states. By default, the export states are always set to 
	// true except for the option eEXPORT_TEXTURE_AS_EMBEDDED. The code below 
	// shows how to change these states.
	IOSETTING_REF.SetBoolProp(EXP_FBX_MATERIAL, true);
	IOSETTING_REF.SetBoolProp(EXP_FBX_TEXTURE, true);
	IOSETTING_REF.SetBoolProp(EXP_FBX_EMBEDDED, pEmbedMedia);
	IOSETTING_REF.SetBoolProp(EXP_FBX_SHAPE, true);
	IOSETTING_REF.SetBoolProp(EXP_FBX_GOBO, true);
	IOSETTING_REF.SetBoolProp(EXP_FBX_ANIMATION, true);
	IOSETTING_REF.SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);

	// Initialize the exporter by providing a filename.
	if (lExporter->Initialize(pFilename, pFileFormat, pManager->GetIOSettings()) == false)
	{
		printf("Call to FbxExporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
		return false;
	}

	FbxManager::GetFileFormatVersion(lMajor, lMinor, lRevision);
	printf("FBX file format version %d.%d.%d\n\n", lMajor, lMinor, lRevision);

	// Export the scene.
	lStatus = lExporter->Export(pScene);

	// Destroy the exporter.
	lExporter->Destroy();
	return lStatus;
}



//void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene)
void InitializeSdkObjects()
{
	//The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
	pManager = FbxManager::Create();
	if (!pManager){
		printf("Error: Unable to create FBX Manager!\n");
		exit(1);
	}
	else printf("Autodesk FBX SDK version %s\n", pManager->GetVersion());

	//Create an IOSettings object. This object holds all import/export settings.
	FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);

	//bool anim = ios->GetBoolProp(EXP_LIGHT, false); // will return true if not found, since we pass true as second param

	//FbxProperty pro = ios->GetProperty(EXP_FBX_EXPORT_FILE_VERSION);
	//ios->SetIntProp(EXP_FBX_EXPORT_FILE_VERSION, 1);
	////FbxDataType type = pro.GetPropertyDataType();
	////printf("%d %s\n", type.GetType(), type.GetName());
	//printf("%d\n", pro.GetEnumCount());
	//for (int i = 0; i < pro.GetEnumCount(); i++) {
	//	printf("%d %s\n", i, pro.GetEnumValue(i));
	//}

	//printf("\n%d\n", pro.Get<int>());


	pManager->SetIOSettings(ios);

	//Load plugins from the executable directory (optional)
	//FbxString lPath = FbxGetApplicationDirectory();
	//pManager->LoadPluginsDirectory(lPath.Buffer());

	//Create an FBX scene. This object holds most objects imported/exported from/to files.
	pScene = FbxScene::Create(pManager, "My Scene");
	if (!pScene){
		printf("Error: Unable to create FBX scene!\n");
		exit(1);
	}
}


void SaveFbxFile(const char* outFile)
{
	SaveScene(pManager, pScene, outFile);
	pScene->Destroy();
	pManager->Destroy();
}


void ExportBones(const Value& obj)
{
	const Value& boneName = obj["BoneName"];
	if (boneName.IsNull())
	{
		return;
	}

	//string name = obj["Name"].GetString();

	const Value& parentIndex = obj["ParentIndex"];
	const Value& boneTransform = obj["BoneTransform"];

	fbxBones.resize(boneName.Size());
	for (uint32_t i = 0; i < fbxBones.size(); i++)
	{
		FbxNode* boneNode = FbxNode::Create(pScene, boneName[i].GetString());
		fbxBones[i] = boneNode;
		double x, y, z, w;
		x = boneTransform[i * 7 + 0].GetDouble();
		y = boneTransform[i * 7 + 1].GetDouble();
		z = boneTransform[i * 7 + 2].GetDouble();
		boneNode->LclTranslation.Set(FbxDouble3(x, y, z));

		x = boneTransform[i * 7 + 3].GetDouble();
		y = boneTransform[i * 7 + 4].GetDouble();
		z = boneTransform[i * 7 + 5].GetDouble();
		w = boneTransform[i * 7 + 6].GetDouble();

		FbxSkeleton* pSkeleton = FbxSkeleton::Create(pScene, boneName[i].GetString());
		int parent = parentIndex[i].GetInt();
		if (parent == -1) {
			pSkeleton->SetSkeletonType(FbxSkeleton::eRoot);
		}
		else {
			pSkeleton->SetSkeletonType(FbxSkeleton::eLimbNode);
			pSkeleton->Size.Set(0.1);
		}

		boneNode->SetNodeAttribute(pSkeleton);
	}

	for (uint32_t i = 0; i < fbxBones.size(); i++) {
		int parent = parentIndex[i].GetInt();
		if (parent == -1) {
			pScene->GetRootNode()->AddChild(fbxBones[i]);
		}
		else {
			fbxBones[parent]->AddChild(fbxBones[i]);
		}
	}

	FbxPose* lPose = FbxPose::Create(pScene, boneName[0].GetString());
	// default pose type is rest pose, so we need to set the type as bind pose
	lPose->SetIsBindPose(true);

	for (uint32_t i = 0; i < fbxBones.size(); i++) {
		FbxNode*  lKFbxNode = fbxBones[i];
		FbxMatrix lBindMatrix = lKFbxNode->EvaluateGlobalTransform();
		lPose->Add(lKFbxNode, lBindMatrix);
	}

	pScene->AddPose(lPose);

}


void ExportFbxMesh(const Value& obj)
{
	string name = obj["Name"].GetString();

	FbxNode* pNode = FbxNode::Create(pManager, name.c_str());
	FbxMesh* pMesh = FbxMesh::Create(pManager, name.c_str());
	pNode->AddNodeAttribute(pMesh);
	pScene->GetRootNode()->AddChild(pNode);

	int numVertex = obj["NumVertex"].GetInt();
	
	{
		pMesh->InitControlPoints(numVertex);
		FbxVector4* lControlPoints = pMesh->GetControlPoints();
		const Value& pos = obj["Position"];
		for (int i = 0; i < numVertex; i++)
		{
			double x = pos[i * 3 + 0].GetDouble();
			double y = pos[i * 3 + 1].GetDouble();
			double z = pos[i * 3 + 2].GetDouble();
			lControlPoints[i] = FbxVector4(x, y, z);
		}
	}

	{
		FbxGeometryElementNormal* lGeometryElementNormal = pMesh->CreateElementNormal();
		lGeometryElementNormal->SetMappingMode(FbxGeometryElement::eByControlPoint);
		lGeometryElementNormal->SetReferenceMode(FbxGeometryElement::eDirect);
		FbxLayerElementArrayTemplate<FbxVector4>& array = lGeometryElementNormal->GetDirectArray();

		const Value& normal = obj["Normal"];
		for (int i = 0; i < numVertex; i++)
		{
			double x = normal[i * 3 + 0].GetDouble();
			double y = normal[i * 3 + 1].GetDouble();
			double z = normal[i * 3 + 2].GetDouble();
			array.Add(FbxVector4(x, y, z));
		}
	}

	{
		FbxGeometryElementUV* lUVDiffuseElement = pMesh->CreateElementUV("DiffuseUV");
		FBX_ASSERT(lUVDiffuseElement != NULL);
		lUVDiffuseElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
		lUVDiffuseElement->SetReferenceMode(FbxGeometryElement::eDirect);
		FbxLayerElementArrayTemplate<FbxVector2>& array = lUVDiffuseElement->GetDirectArray();

		const Value& v = obj["UV0"];
		for (int i = 0; i < numVertex; i++)
		{
			double x = v[i * 2 + 0].GetDouble();
			double y = v[i * 2 + 1].GetDouble();
			array.Add(FbxVector2(x, y));
		}
	}

	{
		const Value& color = obj["Color"];
		if (!color.IsNull())
		{
			FbxGeometryElementVertexColor* pColorElement = pMesh->CreateElementVertexColor();
			FBX_ASSERT(pColorElement != NULL);
			pColorElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
			pColorElement->SetReferenceMode(FbxGeometryElement::eDirect);
			FbxLayerElementArrayTemplate<FbxColor>& array = pColorElement->GetDirectArray();

			for (int i = 0; i < numVertex; i++)
			{
				double r = color[i * 4 + 0].GetDouble();
				double g = color[i * 4 + 1].GetDouble();
				double b = color[i * 4 + 2].GetDouble();
				double a = color[i * 4 + 3].GetDouble();
				array.Add(FbxColor(r, g, b, a));
			}
		}
	}

	{
		const Value& Indeices = obj["Indeices"];

		for (uint32_t subMesh = 0; subMesh < Indeices.Size(); subMesh++)
		{
			const Value& index0 = Indeices[subMesh];
			int numIndex = index0.Size();
			printf("index %d\n", numIndex);
			for (int i = 0; i < numIndex / 3; i++)
			{
				pMesh->BeginPolygon(-1, -1, subMesh);
				pMesh->AddPolygon(index0[i * 3 + 0].GetInt());
				pMesh->AddPolygon(index0[i * 3 + 1].GetInt());
				pMesh->AddPolygon(index0[i * 3 + 2].GetInt());
				pMesh->EndPolygon();
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

	// export skin
	const Value& boneIndex = obj["BoneIndex"];

	if (!boneIndex.IsNull())
	{
		if (fbxBones.empty())
		{
			printf("no bones, can not export skin");
			return;
		}

		const Value& boneWeight = obj["BoneWeight"];
		vector<FbxCluster*> clusters(fbxBones.size(), NULL);
		for (uint32_t i = 0; i < fbxBones.size(); i++) {
			FbxCluster* pCluster = FbxCluster::Create(pScene, "");
			pCluster->SetLink(fbxBones[i]);
			pCluster->SetLinkMode(FbxCluster::eTotalOne);
			clusters[i] = pCluster;
		}

		for (int i = 0; i < numVertex; i++) {
			for (int j = 0; j < 4; j++) {
				int bone = boneIndex[i * 4 + j].GetInt();
				double weight = boneWeight[i * 4 + j].GetDouble();
				clusters[bone]->AddControlPointIndex(i, weight);
			}
		}

		FbxSkin* lSkin = FbxSkin::Create(pScene, "");

		FbxScene* p = pNode->GetScene();
		FbxAMatrix modelMatrix = pNode->EvaluateGlobalTransform();
		for (uint32_t i = 0; i < clusters.size(); i++) {
			clusters[i]->SetTransformMatrix(modelMatrix);
			FbxAMatrix boneMatrix = fbxBones[i]->EvaluateGlobalTransform();
			clusters[i]->SetTransformLinkMatrix(boneMatrix);
			lSkin->AddCluster(clusters[i]);
		}

		pMesh->AddDeformer(lSkin);
	}

}



FbxNode* FindNode(const string& path)
{
	FbxNode* pNode = pScene->GetRootNode();

	string token;	
	size_t i = 0, j = 0;
	while (true)
	{
		j = path.find_first_of('/', i);
		if (j == string::npos)
		{
			token = path.substr(i);
			pNode = pNode->FindChild(token.c_str());
			return pNode;
		}
		else
		{
			token = path.substr(i, j - i);
			i = j + 1;

			if (token == "Position")
			{
				continue;
			}

			pNode = pNode->FindChild(token.c_str());
			if (pNode == NULL)
			{
				return NULL;
			}
		}
	}

	return NULL;
}



void ExportCurve(FbxAnimLayer* lAnimLayer, FbxNode* pNode, const Value& animCurve, uint32_t begin, uint32_t end)
{
	int rx = -1, ry = -1, rz = -1, rw = -1;
	for (uint32_t i = begin; i < end; i++)
	{
		FbxAnimCurve* fbxCurve = NULL;
		const Value& curve = animCurve[i];
		if (strcmp(curve["propertyName"].GetString(), "m_LocalPosition.x") == 0)
		{
			fbxCurve = pNode->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
		} 
		else if (strcmp(curve["propertyName"].GetString(), "m_LocalPosition.y") == 0)
		{
			fbxCurve = pNode->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
		}
		else if (strcmp(curve["propertyName"].GetString(), "m_LocalPosition.z") == 0)
		{
			fbxCurve = pNode->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);
		}
		else if (strcmp(curve["propertyName"].GetString(), "m_LocalRotation.x") == 0)
		{
			rx = i;
		}
		else if (strcmp(curve["propertyName"].GetString(), "m_LocalRotation.y") == 0)
		{
			ry = i;
		}
		else if (strcmp(curve["propertyName"].GetString(), "m_LocalRotation.z") == 0)
		{
			rz = i;
		}
		else if (strcmp(curve["propertyName"].GetString(), "m_LocalRotation.w") == 0)
		{
			rw = i;
		}

		if (fbxCurve != NULL)
		{
			fbxCurve->KeyModifyBegin();

			FbxTime lTime;
			const Value& time = curve["time"];
			const Value& value = curve["value"];
			for (uint32_t i = 0; i < time.Size(); i++)
			{
				double timeValue = time[i].GetDouble();
				double v = value[i].GetDouble();
				lTime.SetSecondDouble(timeValue);
				int lKeyIndex = fbxCurve->KeyAdd(lTime);
				fbxCurve->KeySetValue(lKeyIndex, v);
				fbxCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);
			}
			fbxCurve->KeyModifyEnd();
		}
	}

	// rotation animation will be convert from quaternion to euler angle
	if (rx != -1)
	{
		FbxAnimCurve* fbxCurve[3];

		fbxCurve[0] = pNode->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
		fbxCurve[1] = pNode->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
		fbxCurve[2] = pNode->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);

		for (int i = 0; i < 3; i++)
		{
			fbxCurve[i]->KeyModifyBegin();
		}

		FbxTime lTime;
		const Value& time = animCurve[rx]["time"];
		for (uint32_t i = 0; i < time.Size(); i++)
		{
			double timeValue = time[i].GetDouble();

			double x = animCurve[rx]["value"][i].GetDouble();
			double y = animCurve[ry]["value"][i].GetDouble();
			double z = animCurve[rz]["value"][i].GetDouble();
			double w = animCurve[rw]["value"][i].GetDouble();
			FbxQuaternion qRot(x, y, z, w);
			FbxVector4 euler = qRot.DecomposeSphericalXYZ();

			for (int j = 0; j < 3; j++)
			{
				lTime.SetSecondDouble(timeValue);
				int lKeyIndex = fbxCurve[j]->KeyAdd(lTime);
				fbxCurve[j]->KeySetValue(lKeyIndex, euler[j]);
				fbxCurve[j]->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);
			}			
		}
		
		for (int i = 0; i < 3; i++)
		{
			fbxCurve[i]->KeyModifyEnd();
		}
	}
}



void ExportAnimation(const Value& obj)
{
	double timeLength = obj["Time"].GetDouble();
	double frameRate = obj["FrameRate"].GetDouble();

	const Value& animCurve = obj["CurveData"];

	FbxAnimStack* lAnimStack = FbxAnimStack::Create(pScene, "base stack");
	FbxAnimLayer* lAnimLayer = FbxAnimLayer::Create(pScene, "Base Layer");
	lAnimStack->AddMember(lAnimLayer);

	uint32_t i = 0, j = 0;
	while (true)
	{
		if (i >= animCurve.Size())
		{
			break;
		}

		const Value& first = animCurve[i];

		j = i + 1;
		while (j < animCurve.Size())
		{
			const Value& last = animCurve[j];
			if (strcmp(first["path"].GetString(), last["path"].GetString()) != 0)
			{
				break;
			}
			j++;
		}

		FbxNode* pNode = FindNode(first["path"].GetString());
		if (pNode == NULL)
		{
			printf("path %s node not found!!!!!!!!!!!!!!!!!!!!\n", first["path"].GetString());
		}
		else
		{
			printf("%d path %s node found %p\n", j - i, first["path"].GetString(), pNode);

			FbxAnimCurveNode* curveNode = pNode->LclRotation.GetCurveNode(lAnimLayer, true);
			if (curveNode)
			{
				unsigned int numChannel = curveNode->GetChannelsCount();
				for (uint32_t n = 0; n < numChannel; n++)
				{
					FbxString name = curveNode->GetChannelName(n);
					printf("%d %s\n", n, (const char*)name);
				}
				return;
			}
			

			ExportCurve(lAnimLayer, pNode, animCurve, i, j);
		}

		i = j;
	}

}


bool Export(const char* data, const char* outFile)
{
	Document document;
	document.Parse(data);

	if (document.HasParseError()) {
		ParseErrorCode err = document.GetParseError();
		printf("pares error! %d %d", err, document.GetErrorOffset());
		return false;
	}

	if (!document.IsObject())
	{
		printf("error, not a object");
		return false;
	}

	InitializeSdkObjects();

	const Value& skel = document["Skeleton"];
	if (!skel.IsNull())
	{
		ExportBones(skel);
	}

	const Value& mesh = document["Mesh"];
	if (!mesh.IsNull())
	{
		for (uint32_t i = 0; i < mesh.Size(); i++)
		{
			ExportFbxMesh(mesh[i]);
		}
	}

	const Value& anim = document["Animation"];
	if (!anim.IsNull())
	{
		for (uint32_t i = 0; i < anim.Size(); i++)
		{
			ExportAnimation(anim[i]);
		}
	}
	
	SaveFbxFile(outFile);

	return true;
}


