#ifndef __GTK_CIFROAREA_EXPORTS_H__
#define __GTK_CIFROAREA_EXPORTS_H__

#if defined (_WIN32)
  #if defined (gtkcifroarea_2_0_EXPORTS) || defined (gtkcifroarea_3_0_EXPORTS)
    #define GTK_CIFROAREA_EXPORT __declspec (dllexport)
  #else
    #define GTK_CIFROAREA_EXPORT __declspec (dllimport)
  #endif
#else
  #define GTK_CIFROAREA_EXPORT
#endif

#endif /* __GTK_CIFROAREA_EXPORTS_H__ */
