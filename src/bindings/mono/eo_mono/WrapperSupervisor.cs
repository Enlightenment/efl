/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
using System;
using EventDictionary = System.Collections.Generic.Dictionary<(System.IntPtr desc, object evtDelegate), (System.IntPtr evtCallerPtr, Efl.EventCb evtCaller)>;

namespace Efl.Eo
{

/// <summary>Observe the ownership state  of an Eo wrapper and control its life-cycle.</summary>
internal class WrapperSupervisor
{
    private System.WeakReference weakRef;
#pragma warning disable CS0414
    private Efl.Eo.IWrapper sharedRef;
#pragma warning restore CS0414
    private EventDictionary eoEvents;

    /// <summary>Create a new supervisor for the given.</summary>
    /// <param name="obj">Efl object to be supervised.</param>
    internal WrapperSupervisor(Efl.Eo.IWrapper obj)
    {
        weakRef = new WeakReference(obj);
        sharedRef = null;
        eoEvents = new EventDictionary();
    }

    /// <summary>Efl object being supervised.</summary>
    internal Efl.Eo.IWrapper Target
    {
        get
        {
            return (Efl.Eo.IWrapper) weakRef.Target;
        }
    }

    /// <summary>Dictionary that holds the events related with the supervised object.</summary>
    internal EventDictionary EoEvents
    {
        get
        {
            return eoEvents;
        }
    }

    /// <summary>To be called when the object is uniquely owned by C#, removing its strong reference and making it available to garbage collection.</summary>
    internal void MakeUnique()
    {
        sharedRef = null;
    }

    /// <summary>To be called when the object is owned in the native library too, adding a strong reference to it and making it unavailable for garbage collection.</summary>
    internal void MakeShared()
    {
        if (this.Target == null)
            throw new InvalidOperationException("Tried to make a null reference shared.");
        sharedRef = this.Target;
    }
}

}

