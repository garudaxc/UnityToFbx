# UnityToFbx
load unity asset form asset bundle and convert to fbx

Some time, we want to explore assets in asset bundle. There is tools out there (AssetsBundleExtractor, disunity, etc.).
But they do not always work. The more reliable way is to use unity's function for itself. So, here comes UnityToFbx.

This is how it works:
	
	1.Load GameObject from asset bundle and create instance.
	2.Read mesh data and serialize to json string.
	3.Convert json string to Fbx file using a native dll library.
  
Currently support skin mesh, skeleton and legacy animation clips.

Need Autodesk Fbx sdk to compile dll library.	
	http://www.autodesk.com/products/fbx/overview
