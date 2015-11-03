using System;
using System.Collections.Generic;
using System.Text;
using UnityEngine;
using System.IO;
using System.Collections;


public class BundleLoader : MonoBehaviour
{
    public TextAsset ReadList;

    public List<Texture> textures = new List<UnityEngine.Texture>();
    public List<TextAsset> textAssets = new List<UnityEngine.TextAsset>();
    public List<Material> materials = new List<UnityEngine.Material>();
    public List<GameObject> gameobjects = new List<UnityEngine.GameObject>();
    public List<RuntimeAnimatorController> animation = new List<RuntimeAnimatorController>();

    //public List<Shader> shaders = new List<UnityEngine.Shader>();    

    class ReadAsset
    {
        public string bundle;
        public List<string> assets = new List<string>();
    }

    List<ReadAsset> assetsToRead = new List<ReadAsset>();


    public void LoadAssetBundle()
    {
        for (int i = 0; i < assetsToRead.Count; i++)
        {
            byte[] src = File.ReadAllBytes(assetsToRead[i].bundle);

            AssetBundle b = AssetBundle.CreateFromMemoryImmediate(src);

            for (int j = 0; j < assetsToRead[i].assets.Count; j++)
            {
                string assetName = assetsToRead[i].assets[j];
                string ext = assetName.Substring(assetName.LastIndexOf('.'));

                if (ext == ".fbx")
                {
                    GameObject go = b.LoadAsset<GameObject>(assetName);
                    GameObject inst = GameObject.Instantiate(go);

                    gameobjects.Add(inst);
                    Debug.Log(inst);
                }
                else if (ext == ".png" || ext == ".tga")
                {
                    Texture t = b.LoadAsset<Texture>(assetName);
                    this.textures.Add(t);
                }
                else if (ext == ".shader")
                {
                    //Shader s = b.LoadAsset<Shader>(assetName);
                    //this.shaders.Add(s);
                }
                else if (ext == ".txt")
                {
                    TextAsset t = b.LoadAsset<TextAsset>(assetName);
                    this.textAssets.Add(t);
                }
                else if (ext == ".mat")
                {
                    Material mat = b.LoadAsset<Material>(assetName);
                    this.materials.Add(mat);
                }
                else if (ext == ".controller")
                {
                    RuntimeAnimatorController anim = b.LoadAsset<RuntimeAnimatorController>(assetName);
                    this.animation.Add(anim);

                    Animator a = gameobjects[0].GetComponent<Animator>();
                    if (a != null)
                    {
                        a.runtimeAnimatorController = RuntimeAnimatorController.Instantiate<RuntimeAnimatorController>(anim);                        
                    }

                }
            }

            b.Unload(false);
        }
    }

    public void ReadFileList()
    {
        ReadAsset assetsName = null;

        MemoryStream stream = new MemoryStream(ReadList.bytes);   
        StreamReader reader = new StreamReader(stream); 

        while (true)
        {
            string s = reader.ReadLine();
            if (s == null)
            {
                if (assetsName != null)
                {
                    assetsToRead.Add(assetsName);
                }
                break;
            }
            
            if (s == "")
            {
                if (assetsName != null)
                {
                    assetsToRead.Add(assetsName);
                }
                assetsName = null;
                continue;
            }
            
            if (s[0] == '/' && s[1] == '/')
            {
                continue;
            }

            if (assetsName == null)
            {
                assetsName = new ReadAsset();
                assetsName.bundle = "Assets/" + s;
            }
            else
            {
                assetsName.assets.Add(s);
            }
        }
    }
    

}
