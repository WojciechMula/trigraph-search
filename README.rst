================================================================================
                            Trigram search experiments
================================================================================

This repository contains sample programs for trigram search experiments.

**Problem**: There is a text collection, we want to find all items
containing given word(s).

**Trigram index works as follows**. First the index is built. Each item of
the collection is split into trigrams; for example trigrams for word
'search' are: 'sea', 'ear', 'rch'.  Then then index to collection item is
recorded in them bitmap associated with given trigram.

When we search for a word, the word is split into trigrams. Then all maps
associated with these are fetched, and bit-and together. The final bitmap
points to items having all trigrams from the word, but not necessary the
word itself. An additional step is needed to properly filter the result.

