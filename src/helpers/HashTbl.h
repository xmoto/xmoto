#ifndef __HASHTBL_H__
#define __HASHTBL_H__

#include "VExcept.h"
#include <string>
#include <vector>

template<typename _T>
struct HashTblEntry {
  std::string Name;
  _T Value;
  HashTblEntry *pNext;
};

template<typename _T>
class HashTbl {
public:
  HashTbl() {
    /* Reset counter */
    m_nNumEntries = 0;

    /* Clear table */
    for (int i = 0; i < 0x4000; i++)
      m_Tbl[i] = NULL;
  }

  ~HashTbl() {
    /* Clear table */
    clear();
  }

  /* Methods */
  std::vector<_T> enumEntries(void) {
    /* Create vector of all entries in table */
    std::vector<_T> Ret;
    int nHash = 0, nRem = m_nNumEntries;
    while (nHash < 0x4000 && nRem > 0) {
      HashTblEntry<_T> *p = m_Tbl[nHash];
      while (p != NULL && nRem > 0) {
        Ret.push_back(p->Value);
        nRem--;
        p = p->pNext;
      }

      /* Next hash */
      nHash++;
    }
    return Ret;
  }

  void addEntry(const std::string &Name, _T Value) {
    /* Add entry to table - first determine it's hash */
    unsigned short nHash = _HashName(Name);

    /* Go through linked list to see if we already got this one */
    for (HashTblEntry<_T> *p = m_Tbl[nHash]; p != NULL; p = p->pNext) {
      /* This one? */
      if (p->Name == Name) {
        /* Yeah, error */
        throw Exception("entry name in use");
      }
    }

    /* Okay, we got this far, which means we can safely add the entry */
    HashTblEntry<_T> *pNew = new HashTblEntry<_T>;
    pNew->Name = Name;
    pNew->Value = Value;
    pNew->pNext = m_Tbl[nHash];
    m_Tbl[nHash] = pNew;

    /* Update entry counter */
    m_nNumEntries++;
  }

  void removeEntry(const std::string &Name) {
    if (m_nNumEntries > 0) {
      /* Get rid of entry */
      unsigned short nHash = _HashName(Name);
      HashTblEntry<_T> *pPrev = NULL;

      for (HashTblEntry<_T> *p = m_Tbl[nHash]; p != NULL; p = p->pNext) {
        /* This one? */
        if (p->Name == Name) {
          /* Yeah, delete it */
          if (pPrev != NULL)
            pPrev->pNext = p->pNext;
          else
            m_Tbl[nHash] = p->pNext;

          delete p;

          m_nNumEntries--;
          return;
        }

        pPrev = p;
      }
    }

    /* Nothing found, error */
    throw Exception("entry not found");
  }

  _T getEntry(const std::string &Name) {
    HashTblEntry<_T> *p = _GetEntryByName(Name);
    if (p == NULL)
      throw Exception("entry not found");

    return p->Value;
  }

  bool checkEntry(const std::string &Name) {
    return _GetEntryByName(Name) != NULL;
  }

  int numEntries(void) { return m_nNumEntries; }

  void clear(void) {
    /* Delete everything in table */
    int nHash = 0;
    while (nHash < 0x4000 && m_nNumEntries > 0) {
      HashTblEntry<_T> *p = m_Tbl[nHash];
      while (p != NULL && m_nNumEntries > 0) {
        HashTblEntry<_T> *pNext = p->pNext;
        delete p;
        m_nNumEntries--;
        p = pNext;
      }

      /* Next hash */
      nHash++;
    }
  }

private:
  /* Data */
  int m_nNumEntries;
  HashTblEntry<_T> *m_Tbl[0x4000];

  /* Helpers */
  HashTblEntry<_T> *_GetEntryByName(const std::string &Name) {
    /* Find table entry */
    unsigned short nHash = _HashName(Name);

    for (HashTblEntry<_T> *p = m_Tbl[nHash]; p != NULL; p = p->pNext) {
      /* This one? */
      if (p->Name == Name) {
        /* Yeah */
        return p;
      }
    }

    return NULL;
  }

  unsigned short _HashName(const std::string &Name) {
    /* Determine 14-bit hash of string */
    const unsigned short *pn = (const unsigned short *)Name.c_str();
    unsigned short nHash = 0x349a; /* from brain:/dev/random */
    for (int i = 0; i < Name.length() / 2; i++) {
      nHash ^= pn[i];
    }
    return nHash >> 2;
  }
};

#endif
