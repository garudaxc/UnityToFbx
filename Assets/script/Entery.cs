using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using LitJson;
using System.Text;


public class Entery : MonoBehaviour
{   

    IEnumerator LogAsset()
    {
        //StreamWriter stream = File.CreateText("D:/test/apkpure/filelist.txt");
        //string[] files = Directory.GetFiles("D:/test/apkpure/out/");


        StreamWriter stream = File.CreateText("D:/test/filelist2.txt");
        string[] files = Directory.GetFiles("D:/test/lib/base.apk/assets/bin/Data");       

        Debug.Log(files.Length);
        for (int i = 0; i < files.Length; i++)
        {
            try
            {
                stream.WriteLine(files[i].Substring(files[i].LastIndexOf('/') + 1));
            }
            catch (System.Exception ex)
            {
                Debug.Log(ex);
                break;
            }

            byte[] src = File.ReadAllBytes(files[i]);

            AssetBundleCreateRequest assetBundleCreateRequest = AssetBundle.CreateFromMemory(src);
            yield return assetBundleCreateRequest;

            AssetBundle b = assetBundleCreateRequest.assetBundle;
            string[] assetNames = b.GetAllAssetNames();

            for (int j = 0; j < assetNames.Length; j++)
            {
                stream.WriteLine(assetNames[j]);
            }
            b.Unload(true);

            stream.WriteLine();
        }
        stream.Close();

        Debug.Log("ok");
    }
    

    void Start()
    {
    }        

    // Update is called once per frame
    void Update()
    {

    }

    
}
