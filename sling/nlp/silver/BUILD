package(default_visibility = ["//visibility:public"])

cc_library(
  name = "idf",
  srcs = ["idf.cc"],
  hdrs = ["idf.h"],
  deps = [
    "//sling/base",
    "//sling/file:repository",
    "//sling/nlp/document",
    "//sling/nlp/document:fingerprinter",
    "//sling/task:accumulator",
    "//sling/task:documents",
    "//sling/util:unicode",
  ],
  alwayslink = 1,
)

cc_library(
  name = "chart",
  srcs = ["chart.cc"],
  hdrs = ["chart.h"],
  deps = [
    "//sling/frame:store",
    "//sling/nlp/document",
    "//sling/nlp/kb:phrase-table",
  ],
)

cc_library(
  name = "mentions",
  srcs = ["mentions.cc"],
  hdrs = ["mentions.h"],
  deps = [
    ":chart",
    ":idf",
    "//sling/frame:object",
    "//sling/frame:serialization",
    "//sling/frame:store",
    "//sling/nlp/document",
    "//sling/nlp/document:annotator",
    "//sling/nlp/document:fingerprinter",
    "//sling/nlp/document:phrase-tokenizer",
    "//sling/nlp/kb:calendar",
    "//sling/nlp/kb:facts",
    "//sling/nlp/kb:phrase-table",
    "//sling/nlp/kb:resolver",
  ],
  alwayslink = 1,
)

cc_library(
  name = "anaphora",
  srcs = ["anaphora.cc"],
  deps = [
    "//sling/nlp/document",
    "//sling/nlp/document:annotator",
    "//sling/nlp/document:fingerprinter",
    "//sling/nlp/kb:facts",
  ],
  alwayslink = 1,
)

cc_library(
  name = "phrases",
  srcs = ["phrases.cc"],
  deps = [
    ":chart",
    "//sling/frame:serialization",
    "//sling/nlp/document",
    "//sling/nlp/document:annotator",
    "//sling/nlp/document:lex",
    "//sling/nlp/kb:facts",
    "//sling/nlp/kb:phrase-table",
    "//sling/stream:file-input",
    "//sling/util:fingerprint",
    "//sling/util:mutex",
  ],
  alwayslink = 1,
)

cc_library(
  name = "relations",
  srcs = ["relations.cc"],
  deps = [
    "//sling/nlp/document",
    "//sling/nlp/document:annotator",
    "//sling/nlp/kb:facts",
  ],
  alwayslink = 1,
)

cc_library(
  name = "annotators",
  deps = [
    ":mentions",
    ":anaphora",
    ":phrases",
    ":relations",
  ],
)

cc_binary(
  name = "parse-chart",
  srcs = ["parse-chart.cc"],
  deps = [
    ":annotators",
    ":chart",
    ":idf",
    "//sling/base",
    "//sling/file:posix",
    "//sling/file:recordio",
    "//sling/frame:object",
    "//sling/frame:serialization",
    "//sling/nlp/document",
    "//sling/nlp/document:document-tokenizer",
    "//sling/nlp/document:lex",
  ],
)

