// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <program.h>

#include <arpc++/arpc++.h>

#include <flower/cat/configuration.ad.h>
#include <flower/cat/start.h>

using arpc::ArgdataParser;
using flower::cat::Configuration;
using flower::cat::Start;

void program_main(const argdata_t* ad) {
  Configuration configuration;
  {
    ArgdataParser argdata_parser;
    configuration.Parse(*ad, &argdata_parser);
  }
  Start(configuration);
}
