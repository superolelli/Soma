#pragma once

template <class T>
class TSingleton
{
  protected:

  static T *m_pSingleton;   // static object

  public:


  virtual ~TSingleton ()
  {
  }
  

  inline static T* Get ()
  {
    // Is an instance existing already
    if (!m_pSingleton)
	 m_pSingleton = new T;   // No, then make a new one

    // return pointer to instance
    return (m_pSingleton);

  }

 

  static void Del ()
  {
    // Existing instance?
    if (m_pSingleton)
    {
      delete (m_pSingleton);  // Yes -> delete it
      m_pSingleton = nullptr;    // and set the pointer to null
    }

  } 

};


//generating the static variable
template <class T>
T* TSingleton<T>::m_pSingleton = 0;


