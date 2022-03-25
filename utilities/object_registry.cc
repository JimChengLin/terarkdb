// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#include "terarkdb/utilities/object_registry.h"

#include "terarkdb/env.h"
#include "util/logging.h"

namespace TERARKDB_NAMESPACE {
#ifndef ROCKSDB_LITE
// Looks through the "type" factories for one that matches "name".
// If found, returns the pointer to the Entry matching this name.
// Otherwise, nullptr is returned
const ObjectLibrary::Entry *ObjectLibrary::FindEntry(
    const std::string &type, const std::string &name) const {
  auto entries = entries_.find(type);
  if (entries != entries_.end()) {
    for (const auto &entry : entries->second) {
      if (entry->matches(name)) {
        return entry.get();
      }
    }
  }
  return nullptr;
}

void ObjectLibrary::AddEntry(const std::string &type,
                             std::unique_ptr<Entry> &entry) {
  auto &entries = entries_[type];
  entries.emplace_back(std::move(entry));
}

// Returns the Default singleton instance of the ObjectLibrary
// This instance will contain most of the "standard" registered objects
std::shared_ptr<ObjectLibrary> &ObjectLibrary::Default() {
  static std::shared_ptr<ObjectLibrary> instance =
      std::make_shared<ObjectLibrary>();
  return instance;
}

std::shared_ptr<ObjectRegistry> ObjectRegistry::NewInstance() {
  std::shared_ptr<ObjectRegistry> instance = std::make_shared<ObjectRegistry>();
  return instance;
}

ObjectRegistry::ObjectRegistry() {
  libraries_.push_back(ObjectLibrary::Default());
}

// Searches (from back to front) the libraries looking for the
// an entry that matches this pattern.
// Returns the entry if it is found, and nullptr otherwise
const ObjectLibrary::Entry *ObjectRegistry::FindEntry(
    const std::string &type, const std::string &name) const {
  for (auto iter = libraries_.crbegin(); iter != libraries_.crend(); ++iter) {
    const auto *entry = iter->get()->FindEntry(type, name);
    if (entry != nullptr) {
      return entry;
    }
  }
  return nullptr;
}

#endif  // ROCKSDB_LITE
}  // namespace TERARKDB_NAMESPACE
