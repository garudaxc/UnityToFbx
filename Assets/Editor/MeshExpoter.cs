using System;
using System.Collections.Generic;
using System.Text;
using UnityEngine;
using System.IO;
using LitJson;
using UnityEditor;


	class MeshExpoter
	{
        public void Export(SkinnedMeshRenderer skinRender)
        {
            Mesh mesh = skinRender.sharedMesh;
            Mesh staticMesh = new UnityEngine.Mesh();
            skinRender.BakeMesh(staticMesh);

            meshes_.Add(new ExportMesh(mesh, staticMesh));
        }

        public void Export(Transform[] bones)
        {
            if (this.Skeleton != null)
            {
                Debug.LogError("already set a skeleton!");
                return;
            }

            this.Skeleton = new ExportSkeleton(bones);
        }

        public void Export(AnimationClip animClip)
        {
            this.animations_.Add(new ExportAnimation(animClip));
        }

        public void SaveToFile(string path)
        {
            StreamWriter stream = File.CreateText(path);

            string data = SerializeToString();
            stream.WriteLine(data);

            stream.Close();

            Debug.Log("done!");
        }

        public string SerializeToString()
        {
            this.Mesh = meshes_.ToArray();
            this.Animation = animations_.ToArray();

            StringBuilder sb = new StringBuilder();
            JsonWriter writer = new JsonWriter(sb);

            JsonMapper.ToJson(this, writer);

            return sb.ToString();
        }

        public class ExportMesh
        {
            public string Name { get; set; }
            public int NumVertex { get; set; }
            public int NumSubmesh { get; set; }
            public double[] Position { get; set; }
            public double[] Normal { get; set; }
            public double[] UV0 { get; set; }
            public double[] Color { get; set; }
            public int[][] Indeices { get; set; }
            public int[] BoneIndex { get; set; }
            public double[] BoneWeight { get; set; }

            public ExportMesh(Mesh mesh, Mesh staticMesh)
            {
                Name = mesh.name;
                NumVertex = mesh.vertexCount;
                NumSubmesh = mesh.subMeshCount;
                
                Position = new double[NumVertex * 3];
                for (int i = 0; i < NumVertex; i++)
                {
                    Position[i * 3 + 0] = staticMesh.vertices[i].x;
                    Position[i * 3 + 1] = staticMesh.vertices[i].y;
                    Position[i * 3 + 2] = staticMesh.vertices[i].z;
                }

                Normal = new double[NumVertex * 3];
                for (int i = 0; i < NumVertex; i++)
                {
                    Normal[i * 3 + 0] = staticMesh.normals[i].x;
                    Normal[i * 3 + 1] = staticMesh.normals[i].y;
                    Normal[i * 3 + 2] = staticMesh.normals[i].z;
                }

                UV0 = new double[NumVertex * 2];
                for (int i = 0; i < NumVertex; i++)
                {
                    UV0[i * 2 + 0] = staticMesh.uv[i].x;
                    UV0[i * 2 + 1] = staticMesh.uv[i].y;
                }

                if (staticMesh.colors != null)
                {
                    Color = new double[NumVertex * 4];
                    for (int i = 0; i < NumVertex; i++)
                    {
                        Color[i * 4 + 0] = staticMesh.colors[i].r;
                        Color[i * 4 + 1] = staticMesh.colors[i].g;
                        Color[i * 4 + 2] = staticMesh.colors[i].b;
                        Color[i * 4 + 3] = staticMesh.colors[i].a;
                    }
                }

                Indeices = new int[NumSubmesh][];
                for (int i = 0; i < NumSubmesh; i++)
                {
                    Indeices[i] = staticMesh.GetIndices(i);
                }

                mesh.UploadMeshData(false);
                if (mesh.boneWeights.Length == 0)
                {
                    return;
                }

                BoneIndex = new int[NumVertex * 4];
                for (int i = 0; i < NumVertex; i++)
                {
                    BoneIndex[i * 4 + 0] = mesh.boneWeights[i].boneIndex0;
                    BoneIndex[i * 4 + 1] = mesh.boneWeights[i].boneIndex1;
                    BoneIndex[i * 4 + 2] = mesh.boneWeights[i].boneIndex2;
                    BoneIndex[i * 4 + 3] = mesh.boneWeights[i].boneIndex3;
                }

                BoneWeight = new double[NumVertex * 4];
                for (int i = 0; i < NumVertex; i++)
                {
                    BoneWeight[i * 4 + 0] = mesh.boneWeights[i].weight0;
                    BoneWeight[i * 4 + 1] = mesh.boneWeights[i].weight1;
                    BoneWeight[i * 4 + 2] = mesh.boneWeights[i].weight2;
                    BoneWeight[i * 4 + 3] = mesh.boneWeights[i].weight3;
                }
            }

            void ListBones(Transform bone, int parent, List<int> parentIndex, List<Transform> boneList)
            {

                //List<int> parentIndex = new List<int>();
                //List<Transform> boneList = new List<UnityEngine.Transform>();
                //ListBones(rootBone, -1, parentIndex, boneList);

                int index = parentIndex.Count;
                parentIndex.Add(parent);
                boneList.Add(bone);

                foreach (Transform child in bone.transform)
                {
                    ListBones(child, index, parentIndex, boneList);
                }
            }

        }

        public class ExportSkeleton
        {
            public string[] BoneName { get; set; }
            public int[] ParentIndex { get; set; }
            public double[] BoneTransform { get; set; }

            public ExportSkeleton(Transform[] bones)
            {

                ParentIndex = new int[bones.Length];
                for (int i = 0; i < bones.Length; i++)
                {
                    int parent = -1;
                    for (int j = 0; j < bones.Length; j++)
                    {
                        if (bones[i].parent == bones[j])
                        {
                            parent = j;
                            break;
                        }
                    }
                    ParentIndex[i] = parent;
                }

                BoneName = new string[bones.Length];
                BoneTransform = new double[bones.Length * 7];
                for (int i = 0; i < bones.Length; i++)
                {
                    BoneName[i] = bones[i].name;
                    BoneTransform[i * 7 + 0] = bones[i].localPosition.x;
                    BoneTransform[i * 7 + 1] = bones[i].localPosition.y;
                    BoneTransform[i * 7 + 2] = bones[i].localPosition.z;
                    BoneTransform[i * 7 + 3] = bones[i].localRotation.x;
                    BoneTransform[i * 7 + 4] = bones[i].localRotation.y;
                    BoneTransform[i * 7 + 5] = bones[i].localRotation.z;
                    BoneTransform[i * 7 + 6] = bones[i].localRotation.w;
                }
            }
        }

        public class ExportAnimation
        {
            public class AnimCurveData
            {
                public string path;
                public string propertyName;
                public double[] inTangent { get; set; }
                public double[] outTangent { get; set; }
                public int[] tangentMode { get; set; }
                public double[] time { get; set; }
                public double[] value { get; set; }

                public void Alloc(int numKey)
                {
                    inTangent = new double[numKey];
                    outTangent = new double[numKey];
                    tangentMode = new int[numKey];
                    time = new double[numKey];
                    value = new double[numKey];
                }
            }


            public ExportAnimation(AnimationClip clip)
            {
                Time = clip.length;
                FrameRate = clip.frameRate;

                AnimationClipCurveData[] data = AnimationUtility.GetAllCurves(clip, true);

                CurveData = new AnimCurveData[data.Length];
                for (int i = 0; i < data.Length; i++)
                {
                    CurveData[i] = ExprotCurve(data[i]);
                }
            }

            AnimCurveData ExprotCurve(AnimationClipCurveData data)
            {
                AnimCurveData curve = new AnimCurveData();
                curve.path = data.path;
                curve.propertyName = data.propertyName;

                Keyframe[] keys = data.curve.keys;
                curve.Alloc(keys.Length);
                for (int i = 0; i < keys.Length; i++)
                {
                    curve.inTangent[i] = keys[i].inTangent;
                    curve.outTangent[i] = keys[i].outTangent;
                    curve.tangentMode[i] = keys[i].tangentMode;
                    curve.time[i] = keys[i].time;
                    curve.value[i] = keys[i].value;
                }

                return curve;
            }

            public double Time { get; set; }
            public double FrameRate { get; set; }
            public AnimCurveData[] CurveData { get; set; }
        }
        
        string filePath_;
        List<ExportMesh> meshes_ = new List<ExportMesh>();
        List<ExportAnimation> animations_ = new List<ExportAnimation>();

        public ExportSkeleton Skeleton { get; set; }
        public ExportMesh[] Mesh { get; set; }
        public ExportAnimation[] Animation { get; set; }

	}