# CASPAR frame semantics parser

CASPAR is a frame semantics parser trained on OntoNotes 5 data. Mentions,
entity types (partial), and 
[PropBank](https://propbank.github.io/) semantic role labels are extracted from 
this corpus to produce a frame semantic corpus in SLING document format.

We use the standard [CoNLL-2012](http://conll.cemantix.org/2012/data.html) split
of the data to produce training, development, and test corpora.

## Preparing the training data

The LDC2013T19 OntoNotes 5 corpus is needed to produce the training data for
CASPAR. This is licensed by LDC and you need an LDC license to use the corpus:

https://catalog.ldc.upenn.edu/LDC2013T19

To prepare the training data for the parser, place `LDC2013T19.tar.gz` in 
`local/data/corpora/ontonotes` and run the `make_corpus.sh` script:

```
sling/nlp/parser/ontonotes/make_corpus.sh
```

This script will perform the following steps to produce the training data:

* Unpack OntoNotes 5.
* Download and unpack the CoNLL formated OntoNotes 5 data and tools.
* Generate CoNLL files from the OntoNotes data.
* Convert CoNLL files to SLING format.
* Shuffle the training data.

This will put the training and evaluation data into `local/data/corpora/caspar`:
* `train.rec` contains the training data.
* `dev.rec` contains the developemnt data.
* `test.rec` contains the evaluation data.
* `train_shuffled.rec` contains the shuffled training data.

## Pre-trained word embeddings

The CASPAR parser uses pre-trained word embeddings which can be downloaded from
here:
```
curl http://www.jbox.dk/sling/word2vec-32-embeddings.bin -o /tmp/word2vec-32-embeddings.bin
```

These are 32 dimensional word embeddings trained on news text in
[Mikolov's word2vec format](https://github.com/tmikolov/word2vec/blob/master/word2vec.c).

## Train a CASPAR parser model

The `sling/nlp/parser/tools/train_caspar.py` Python script contains an example of
how to train the CASPAR parser model:

```python
import sling
import sling.flags as flags
import sling.task.workflow as workflow

# Start up workflow system.
flags.parse()
workflow.startup()

# Create worflow.
wf = workflow.Workflow("parser-training")

# Parser trainer inputs and outputs.
training_corpus = wf.resource(
  "local/data/corpora/caspar/train_shuffled.rec",
  format="record/document"
)

evaluation_corpus = wf.resource(
  "local/data/corpora/caspar/dev.rec",
  format="record/document"
)

word_embeddings = wf.resource(
  "local/data/corpora/caspar/word2vec-32-embeddings.bin",
  format="embeddings"
)

parser_model = wf.resource(
  "local/data/e/caspar/caspar.flow",
  format="flow"
)

# Parser trainer task.
trainer = wf.task("caspar-trainer")

trainer.add_params({
  "learning_rate": 1.0,
  "learning_rate_decay": 0.8,
  "clipping": 1,
  "optimizer": "sgd",
  "epochs": 50000,
  "batch_size": 32,
  "rampup": 120,
  "report_interval": 500
})

trainer.attach_input("training_corpus", training_corpus)
trainer.attach_input("evaluation_corpus", evaluation_corpus)
trainer.attach_input("word_embeddings", word_embeddings)
trainer.attach_output("model", parser_model)

# Run parser trainer.
workflow.run(wf)

# Shut down.
workflow.shutdown()
```

This model takes ~90 minutes to train. It will output evaluation metrics
each 500 epochs, and when it is done, the final parser model will be written
to `local/data/e/caspar/caspar.flow`.

If you don't have access to OntoNotes 5, you can download a pre-trained model
from [here](http://www.jbox.dk/sling/caspar.flow).

## Testing the CASPAR parser

SLING comes with a [parsing tool](../../sling/nlp/parser/tools/parse.cc)
for annotating a corpus of documents with frames using a parser model,
benchmarking this annotation process, and optionally evaluating the annotated
frames against supplied gold frames.

This tool takes the following commandline arguments:

*  `--parser` : This should point to a Myelin flow, e.g. one created by the
   parser trainer.
*  If `--text` is specified then the parser is run over the supplied text, and
   prints the annotated frame(s) in text mode, e.g.:
   ```
   $ bazel-bin/sling/nlp/parser/tools/parse \
        --parser local/data/e/caspar/caspar.flow --text="Eric loves Hannah."

    {=#1
      :document
      text: "Eric loves Hannah."
      tokens: [{=#2
        start: 0
        size: 4
      }, {=#3
        start: 5
        size: 5
      }, {=#4
        start: 11
        size: 6
      }, {=#5
        start: 17
        break: 0
      }]
      mention: {=#6
        begin: 0
        evokes: {=#7
          :PERSON
        }
      }
      mention: {=#8
        begin: 1
        evokes: {=#9
          :/pb/predicate
          /pb/ARG0: #7
          /pb/ARG1: {=#10
            :PERSON
          }
        }
      }
      mention: {=#11
        begin: 2
        evokes: #10
      }
    }

   ```
*  If `--benchmark` is specified then the parser is run on the document
   corpus specified via `--corpus`. This corpus should be prepared similarly to
   how the training/dev corpora were created. The processing can be limited to
   the first N documents by specifying `--maxdocs N`.

   ```
   $ bazel-bin/sling/nlp/parser/tools/parse --parser local/data/e/caspar/caspar.flow \
       --benchmark --corpus local/data/corpora/caspar/dev.rec
   [... I sling/nlp/parser/tools/parse.cc:131] Load parser from local/data/e/caspar/caspar.flow
   [... I sling/nlp/parser/tools/parse.cc:140] 34.7227 ms loading parser
   [... I sling/nlp/parser/tools/parse.cc:204] Benchmarking parser on local/data/corpora/caspar/dev.rec
   [... I sling/nlp/parser/tools/parse.cc:227] 9603 documents, 163104 tokens, 7970.69 tokens/sec
   ```

   If `--profile` is specified, the parser will run with profiling
   instrumentation enabled and output a detailed profile report with execution
   timing for each operation in the neural network.

*  If `--evaluate` is specified then the tool expects `--corpora` to specify
   a corpora with gold frames. It then runs the parser model over a frame-less
   version of this corpora and evaluates the annotated frames vs the gold
   frames. Again, one can use `--maxdocs` to limit the evaluation to the first N
   documents.
   ```
   $ bazel-bin/sling/nlp/parser/tools/parse --parser local/data/e/caspar/caspar.flow \
       --evaluate --corpus local/data/corpora/caspar/dev.rec
   [... I sling/nlp/parser/tools/parse.cc:131] Load parser from local/data/e/caspar/caspar.flow
   [... I sling/nlp/parser/tools/parse.cc:140] 34.7368 ms loading parser
   [... I sling/nlp/parser/tools/parse.cc:235] Evaluating parser on local/data/corpora/caspar/dev.rec
   SPAN_P+=76898
   SPAN_P-=6800
   SPAN_R+=76898
   SPAN_R-=6192
   SPAN_Precision=91.8755
   SPAN_Recall=92.5478
   SPAN_F1=92.2105
   FRAME_P+=77866
   FRAME_P-=5859
   FRAME_R+=77859
   FRAME_R-=5233
   FRAME_Precision=93.0021
   FRAME_Recall=93.7022
   FRAME_F1=93.3508
   TYPE_P+=74277
   TYPE_P-=9448
   TYPE_R+=74275
   TYPE_R-=8817
   TYPE_Precision=88.7154
   TYPE_Recall=89.3889
   TYPE_F1=89.0509
   ROLE_P+=37762
   ROLE_P-=16848
   ROLE_R+=37755
   ROLE_R-=16397
   ROLE_Precision=69.1485
   ROLE_Recall=69.7204
   ROLE_F1=69.4333
   LABEL_P+=0
   LABEL_P-=0
   LABEL_R+=0
   LABEL_R-=0
   LABEL_Precision=0
   LABEL_Recall=0
   LABEL_F1=0
   SLOT_P+=112039
   SLOT_P-=26296
   SLOT_R+=112030
   SLOT_R-=25214
   SLOT_Precision=80.9911
   SLOT_Recall=81.6283
   SLOT_F1=81.3085
   COMBINED_P+=266803
   COMBINED_P-=38955
   COMBINED_R+=266787
   COMBINED_R-=36639
   COMBINED_Precision=87.2595
   COMBINED_Recall=87.9249
   COMBINED_F1=87.591
   #GOLDEN_SPANS=83090
   #PREDICTED_SPANS=83698
   #GOLDEN_FRAMES=83092
   #PREDICTED_FRAMES=83725
   ```

## Using the CASPAR parser in Python

You can use the parser in Python by using the `Parser` class in the Python SLING
API, e.g.:
```
import sling

parser = sling.Parser("local/data/e/caspar/caspar.flow")

text = input("text: ")
doc = parser.parse(text)
print(doc.frame.data(pretty=True))
for m in doc.mentions:
  print("mention", doc.phrase(m.begin, m.end))
```

## Using the CASPAR parser in C++

SLING has a C++ API for the parser. The model can be loaded and initialized in
the following way:

```c++
#include "sling/frame/store.h"
#include "sling/nlp/document/document-tokenizer.h"
#include "sling/nlp/parser/parser.h"

// Load parser model.
sling::Store commons;
sling::nlp::Parser parser;
parser.Load(&commons, "local/data/e/caspar/caspar.flow");
commons.Freeze();

// Create document tokenizer.
sling::nlp::DocumentTokenizer tokenizer;
```

In order to parse some text, it first needs to be tokenized. The document with
text, tokens, and frames is stored in a local document frame store.

```c++
// Create frame store for document.
sling::Store store(&commons);
sling::nlp::Document document(&store);

// Tokenize text.
string text = "Eric loves Hannah.";
tokenizer.Tokenize(&document, text);

// Parse document.
parser.Parse(&document);
document.Update();

// Output document annotations.
std::cout << sling::ToText(document.top(), 2);
```

