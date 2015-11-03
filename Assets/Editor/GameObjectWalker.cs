using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;
using UnityEngine;


class GameObjectEnumerator : IEnumerator
{
    GameObject go_;
    Queue<GameObject> queue_ = new Queue<UnityEngine.GameObject>();

    public GameObjectEnumerator(GameObject go)
    {
        go_ = go;
    }

    public object Current
    {
        get
        {
            return queue_.Peek();
        }
    }

    public bool MoveNext()
    {
        if (queue_.Count == 0)
        {
            queue_.Enqueue(go_);
            return true;
        }

        GameObject go = queue_.Peek();
        foreach (Transform t in go.transform)
        {
            queue_.Enqueue(t.gameObject);
        }

        if (queue_.Count == 1)
        {
            return false;
        }

        queue_.Dequeue();
        return true;
    }

    public void Reset()
    {
        queue_.Clear();
    }
}

class GameObjectWalker : IEnumerable
{
    GameObject go_;
    public GameObjectWalker(GameObject go)
    {
        go_ = go;
    }

    IEnumerator IEnumerable.GetEnumerator()
    {
        return new GameObjectEnumerator(go_);
    }
}