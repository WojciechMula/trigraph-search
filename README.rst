================================================================================
                            Trigram search experiments
================================================================================

This repository contains sample programs for trigram search experiments.

**Problem**: There is a text collection, we want to find all items
containing given word(s).


Trigram search - introduction
------------------------------------------------------------

**A trigram** is a sequence of three adjacent characters from a word.
For instance, the word "thread" consists following trigrams:

* thr,
* hre,
* rea,
* ead.

In the trigram searching we use **an index**, that for each known
trigram that exist in the data, stores the list of indices to
data items that contain given trigram.

During search, the searched phrase is split into trigrams and these
trigrams are used to query the trigram index. After getting the indices,
it is necessary to test whether the selected data items really contains
the searched phrase.  In simple words, the trigram index is used to
**limit** the number of exact searches.

This approach has one obvious advantage: if the index doesn't contain
information about some trigram we immediately know that the whole data
set doesn't contain that phrase.  But when we can fetch indices for all
trigrams, there are two major approaches:

#. Pick list of indices with the smallest number of items.
#. Join all lists (perform a kind of bitwise and) and use
   the result.

I described the first method in separate article `Speeding up LIKE
'%text%' queries`__; it worked very well for an on-disc database.

__ http://0x80.pl/articles/sql-ngram-index.html

It seems that the second method fits better for in-memory datasets, so this
approach is discussed below.


Representation of lists
------------------------------------------------------------

Following data structures are tested:

* `Roaring bitmaps`__;
* plain ``std::vector<uint32_t>``;
* custom bitvector in three variants:

  * ``naive``  - a plain array of words;
  * ``sparse`` - an array of fixed-length arrays of words;
    the subarrays are allocated on demand;
  * ``tracking`` - a plain array of words, but keeping track
    of the first and the last non-zero word in the array.

__ http://roaringbitmap.org/


Test results
------------------------------------------------------------

Below are preliminary results from my computer, just to give
a hint about performance differences. Both the code and the
test suite must be tackled to make numbers more valuable.

+-------------+------------------+---------------------+
| bitvector   | index build [ms] | 10,000 queries [ms] |
+=============+==================+=====================+
| roaring     |             1696 |                 444 |
+-------------+------------------+---------------------+
| std::vector |              762 |                 435 |
+-------------+------------------+---------------------+
| bv naive    |             1524 |                 665 |
+-------------+------------------+---------------------+
| bv tracking |             1418 |                 517 |
+-------------+------------------+---------------------+
| bv naive    |             1394 |                1032 |
+-------------+------------------+---------------------+
