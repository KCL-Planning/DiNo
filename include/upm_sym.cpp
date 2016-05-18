/*
* DiNo Release 1.0
* Copyright (C) 2015: W. Piotrowski, M. Fox, D. Long, D. Magazzeni, F. Mercorio
*
* Read the file "LICENSE.txt" distributed with these sources, or call
* DiNo with the -l switch for additional information.
* Contact: (wiktor.piotrowski@kcl.ac.uk)
*
*/

void
SymmetryClass::SetBestResult(int i, state * temp)
{
  if (!BestInitialized) {
    BestPermutedState = *temp;
    BestInitialized = TRUE;
  } else {
    switch (StateCmp(temp, &BestPermutedState)) {
      case -1:
        Perm.Add(i);
        BestPermutedState = *temp;
        break;
      case 1:
        Perm.Remove(i);
        break;
      case 0:
        // do nothing
        break;
      default:
        Error.Error("funny return value from StateCmp");
    }
  }
}

void state::Normalize()
{
  static SymmetryClass symmetry;

  symmetry.Normalize(this);
}

void state::MultisetSort()
{
  static SymmetryClass symmetry;

  symmetry.MultisetSort(this);
}
