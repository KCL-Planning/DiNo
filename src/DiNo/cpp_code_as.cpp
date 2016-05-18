/*
* DiNo Release 1.0
* Copyright (C) 2015: W. Piotrowski, M. Fox, D. Long, D. Magazzeni, F. Mercorio
*
* Read the file "LICENSE.txt" distributed with these sources, or call
* DiNo with the -l switch for additional information.
* Contact: (wiktor.piotrowski@kcl.ac.uk)
*
*/


#include "mu.hpp"

void
arraytypedecl::generate_assign()
{
  char indexstr[3];

  strcpy(indexstr, (indextype->getsize() > 1 ? "i" : "0"));
  fprintf(codefile,
          "  %s& operator= (const %s& from)\n" "  {\n", mu_name, mu_name);
  if (indextype->getsize() > 1)
    fprintf(codefile,
            "    for (int i = 0; i < %d; i++)\n", indextype->getsize());
  if (elementtype->issimple())
    fprintf(codefile,
            "      array[%s].value(from.array[%s].value());\n",
            indexstr, indexstr);
  else
    fprintf(codefile,
            "      array[%s] = from.array[%s];\n", indexstr, indexstr);
  fprintf(codefile, "    return *this;\n" "  }\n\n");
};

void multisettypedecl::generate_assign()
{
  char indexstr[3];

  strcpy(indexstr, (maximum_size > 1 ? "i" : "0"));
  fprintf(codefile,
          "  %s& operator= (const %s& from)\n" "  {\n", mu_name, mu_name);
  if (maximum_size > 1)
    fprintf(codefile, "    for (int i = 0; i < %d; i++)\n", maximum_size);
  if (elementtype->issimple())
    fprintf(codefile,
            "    {\n"
            "        array[%s].value(from.array[%s].value());\n"
            "        valid[%s].value(from.valid[%s].value());\n",
            indexstr, indexstr, indexstr, indexstr);
  else
    fprintf(codefile,
            "    {\n"
            "       array[%s] = from.array[%s];\n"
            "       valid[%s].value(from.valid[%s].value());\n",
            indexstr, indexstr, indexstr, indexstr);
  fprintf(codefile,
          "    };\n"
          "    current_size = from.get_current_size();\n"
          "    return *this;\n" "  }\n\n");
};

void recordtypedecl::generate_assign()
{
  ste *f;

  fprintf(codefile,
          "  %s& operator= (const %s& from) {\n", mu_name, mu_name);
  for (f = fields; f != NULL; f = f->getnext()) {
    if (f->getvalue()->gettype()->issimple())
      fprintf(codefile,
              "    %s.value(from.%s.value());\n",
              f->getvalue()->generate_code(),
              f->getvalue()->generate_code());
    else
      fprintf(codefile,
              "    %s = from.%s;\n",
              f->getvalue()->generate_code(),
              f->getvalue()->generate_code());
  }
  fprintf(codefile, "    return *this;\n" "  };\n");
};

