using UnityEditor;
using UnityEngine;
using System.Runtime.InteropServices;
using System.Collections.Generic;

//自定义Tset脚本
[CustomEditor(typeof(Entery))]

[ExecuteInEditMode]
//请继承Editor
public class UnityToFbxEditor : Editor
{
    GameObject model_;

    AnimationClip animClip_;
    
    [DllImport("JsonToFbxDll")]
    public static extern int ExportToFbx(string data, string outfile);

    
    //在这里方法中就可以绘制面板。
    public override void OnInspectorGUI()
    {
        //得到Test对象
        Entery entry = (Entery)target;
        BundleLoader bundleLoader_ = entry.GetComponent<BundleLoader>();

        model_ = EditorGUILayout.ObjectField("Model", model_, typeof(GameObject), true) as GameObject;

        animClip_ = EditorGUILayout.ObjectField("AnimClip", animClip_, typeof(AnimationClip), true) as AnimationClip;
        
        if (GUILayout.Button("test"))
        {

            AnimationClip c = animClip_;
            Debug.Log(c.length);
            Debug.Log(c.name);
            Debug.Log(c.GetType());

            AnimationClipCurveData[] data = AnimationUtility.GetAllCurves(animClip_, true);
            Debug.Log(data.Length);

            EditorCurveBinding[] binds = AnimationUtility.GetCurveBindings(c);
            Debug.Log(binds.Length);
        }

        if (GUILayout.Button("load assetbundle"))
        {
            bundleLoader_.ReadFileList();
            bundleLoader_.LoadAssetBundle();
        }

        if (GUILayout.Button("导出文件"))
        {
            if (model_ == null)
            {
                Debug.LogError("model not set!");
                return;
            }

            MeshExpoter expoter = new MeshExpoter();
            Transform[] bones = null;
            
            foreach (GameObject g in new GameObjectWalker(model_))
            {
                Mesh mesh = null;
                SkinnedMeshRenderer skinRenderer = g.GetComponent<SkinnedMeshRenderer>();

                if (skinRenderer != null)
                {
                    expoter.Export(skinRenderer);
                    mesh = skinRenderer.sharedMesh;
                    bones = skinRenderer.bones;
                }

                //if(mesh == null)
                //{
                //    MeshFilter meshFilter = g.GetComponent<MeshFilter>();
                //    mesh = meshFilter != null ? meshFilter.mesh : null;               
                //}

                if (mesh == null)
                {
                    continue;
                }
            }

            if (bones != null) {
                expoter.Export(bones);
            }

            if (animClip_ != null) {
                expoter.Export(animClip_);
            }

            //expoter.SaveToFile(OutputFile_ + model_.name + ".json");

            string data = expoter.SerializeToString();
            string fileName = "Assets/" + model_.name + ".fbx";
            ExportToFbx(data, fileName);

            Debug.Log("saved to " + fileName);
            AssetDatabase.Refresh();
        }

    }

    void OutputAnimationClip(AnimationClipCurveData curveData)
    {
        Debug.Log(curveData.path);
        Debug.Log(curveData.propertyName);

        AnimationCurve curve = curveData.curve;
        Keyframe[] keys = curve.keys;
        Debug.Log(keys.Length);

        Keyframe k = keys[0];
        Debug.Log(k.tangentMode);
    }
}