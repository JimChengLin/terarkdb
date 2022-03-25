// Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).
//
// This file implements the "bridge" between Java and C++ and enables
// calling C++ TERARKDB_NAMESPACE::SstFileWriter methods
// from Java side.

#include <jni.h>
#include <string>

#include "include/org_rocksdb_SstFileWriter.h"
#include "terarkdb/comparator.h"
#include "terarkdb/env.h"
#include "terarkdb/options.h"
#include "terarkdb/sst_file_writer.h"
#include "rocksjni/portal.h"

/*
 * Class:     org_rocksdb_SstFileWriter
 * Method:    newSstFileWriter
 * Signature: (JJJB)J
 */
jlong Java_org_rocksdb_SstFileWriter_newSstFileWriter__JJJB(
    JNIEnv * /*env*/, jclass /*jcls*/, jlong jenvoptions, jlong joptions,
    jlong jcomparator_handle, jbyte jcomparator_type) {
  TERARKDB_NAMESPACE::Comparator *comparator = nullptr;
  switch (jcomparator_type) {
    // JAVA_COMPARATOR
    case 0x0:
      comparator = reinterpret_cast<TERARKDB_NAMESPACE::ComparatorJniCallback *>(
          jcomparator_handle);
      break;

    // JAVA_DIRECT_COMPARATOR
    case 0x1:
      comparator = reinterpret_cast<TERARKDB_NAMESPACE::DirectComparatorJniCallback *>(
          jcomparator_handle);
      break;

    // JAVA_NATIVE_COMPARATOR_WRAPPER
    case 0x2:
      comparator = reinterpret_cast<TERARKDB_NAMESPACE::Comparator *>(jcomparator_handle);
      break;
  }
  auto *env_options =
      reinterpret_cast<const TERARKDB_NAMESPACE::EnvOptions *>(jenvoptions);
  auto *options = reinterpret_cast<const TERARKDB_NAMESPACE::Options *>(joptions);
  TERARKDB_NAMESPACE::SstFileWriter *sst_file_writer =
      new TERARKDB_NAMESPACE::SstFileWriter(*env_options, *options, comparator);
  return reinterpret_cast<jlong>(sst_file_writer);
}

/*
 * Class:     org_rocksdb_SstFileWriter
 * Method:    newSstFileWriter
 * Signature: (JJ)J
 */
jlong Java_org_rocksdb_SstFileWriter_newSstFileWriter__JJ(JNIEnv * /*env*/,
                                                          jclass /*jcls*/,
                                                          jlong jenvoptions,
                                                          jlong joptions) {
  auto *env_options =
      reinterpret_cast<const TERARKDB_NAMESPACE::EnvOptions *>(jenvoptions);
  auto *options = reinterpret_cast<const TERARKDB_NAMESPACE::Options *>(joptions);
  TERARKDB_NAMESPACE::SstFileWriter *sst_file_writer =
      new TERARKDB_NAMESPACE::SstFileWriter(*env_options, *options);
  return reinterpret_cast<jlong>(sst_file_writer);
}

/*
 * Class:     org_rocksdb_SstFileWriter
 * Method:    open
 * Signature: (JLjava/lang/String;)V
 */
void Java_org_rocksdb_SstFileWriter_open(JNIEnv *env, jobject /*jobj*/,
                                         jlong jhandle, jstring jfile_path) {
  const char *file_path = env->GetStringUTFChars(jfile_path, nullptr);
  if (file_path == nullptr) {
    // exception thrown: OutOfMemoryError
    return;
  }
  TERARKDB_NAMESPACE::Status s =
      reinterpret_cast<TERARKDB_NAMESPACE::SstFileWriter *>(jhandle)->Open(file_path);
  env->ReleaseStringUTFChars(jfile_path, file_path);

  if (!s.ok()) {
    TERARKDB_NAMESPACE::RocksDBExceptionJni::ThrowNew(env, s);
  }
}

/*
 * Class:     org_rocksdb_SstFileWriter
 * Method:    put
 * Signature: (JJJ)V
 */
void Java_org_rocksdb_SstFileWriter_put__JJJ(JNIEnv *env, jobject /*jobj*/,
                                             jlong jhandle, jlong jkey_handle,
                                             jlong jvalue_handle) {
  auto *key_slice = reinterpret_cast<TERARKDB_NAMESPACE::Slice *>(jkey_handle);
  auto *value_slice = reinterpret_cast<TERARKDB_NAMESPACE::Slice *>(jvalue_handle);
  TERARKDB_NAMESPACE::Status s = reinterpret_cast<TERARKDB_NAMESPACE::SstFileWriter *>(jhandle)->Put(
      *key_slice, *value_slice);
  if (!s.ok()) {
    TERARKDB_NAMESPACE::RocksDBExceptionJni::ThrowNew(env, s);
  }
}

/*
 * Class:     org_rocksdb_SstFileWriter
 * Method:    put
 * Signature: (JJJ)V
 */
void Java_org_rocksdb_SstFileWriter_put__J_3B_3B(JNIEnv *env, jobject /*jobj*/,
                                                 jlong jhandle, jbyteArray jkey,
                                                 jbyteArray jval) {
  jbyte *key = env->GetByteArrayElements(jkey, nullptr);
  if (key == nullptr) {
    // exception thrown: OutOfMemoryError
    return;
  }
  TERARKDB_NAMESPACE::Slice key_slice(reinterpret_cast<char *>(key),
                           env->GetArrayLength(jkey));

  jbyte *value = env->GetByteArrayElements(jval, nullptr);
  if (value == nullptr) {
    // exception thrown: OutOfMemoryError
    env->ReleaseByteArrayElements(jkey, key, JNI_ABORT);
    return;
  }
  TERARKDB_NAMESPACE::Slice value_slice(reinterpret_cast<char *>(value),
                             env->GetArrayLength(jval));

  TERARKDB_NAMESPACE::Status s = reinterpret_cast<TERARKDB_NAMESPACE::SstFileWriter *>(jhandle)->Put(
      key_slice, value_slice);

  env->ReleaseByteArrayElements(jkey, key, JNI_ABORT);
  env->ReleaseByteArrayElements(jval, value, JNI_ABORT);

  if (!s.ok()) {
    TERARKDB_NAMESPACE::RocksDBExceptionJni::ThrowNew(env, s);
  }
}

/*
 * Class:     org_rocksdb_SstFileWriter
 * Method:    merge
 * Signature: (JJJ)V
 */
void Java_org_rocksdb_SstFileWriter_merge__JJJ(JNIEnv *env, jobject /*jobj*/,
                                               jlong jhandle, jlong jkey_handle,
                                               jlong jvalue_handle) {
  auto *key_slice = reinterpret_cast<TERARKDB_NAMESPACE::Slice *>(jkey_handle);
  auto *value_slice = reinterpret_cast<TERARKDB_NAMESPACE::Slice *>(jvalue_handle);
  TERARKDB_NAMESPACE::Status s =
      reinterpret_cast<TERARKDB_NAMESPACE::SstFileWriter *>(jhandle)->Merge(*key_slice,
                                                                 *value_slice);
  if (!s.ok()) {
    TERARKDB_NAMESPACE::RocksDBExceptionJni::ThrowNew(env, s);
  }
}

/*
 * Class:     org_rocksdb_SstFileWriter
 * Method:    merge
 * Signature: (J[B[B)V
 */
void Java_org_rocksdb_SstFileWriter_merge__J_3B_3B(JNIEnv *env,
                                                   jobject /*jobj*/,
                                                   jlong jhandle,
                                                   jbyteArray jkey,
                                                   jbyteArray jval) {
  jbyte *key = env->GetByteArrayElements(jkey, nullptr);
  if (key == nullptr) {
    // exception thrown: OutOfMemoryError
    return;
  }
  TERARKDB_NAMESPACE::Slice key_slice(reinterpret_cast<char *>(key),
                           env->GetArrayLength(jkey));

  jbyte *value = env->GetByteArrayElements(jval, nullptr);
  if (value == nullptr) {
    // exception thrown: OutOfMemoryError
    env->ReleaseByteArrayElements(jkey, key, JNI_ABORT);
    return;
  }
  TERARKDB_NAMESPACE::Slice value_slice(reinterpret_cast<char *>(value),
                             env->GetArrayLength(jval));

  TERARKDB_NAMESPACE::Status s =
      reinterpret_cast<TERARKDB_NAMESPACE::SstFileWriter *>(jhandle)->Merge(key_slice,
                                                                 value_slice);

  env->ReleaseByteArrayElements(jkey, key, JNI_ABORT);
  env->ReleaseByteArrayElements(jval, value, JNI_ABORT);

  if (!s.ok()) {
    TERARKDB_NAMESPACE::RocksDBExceptionJni::ThrowNew(env, s);
  }
}

/*
 * Class:     org_rocksdb_SstFileWriter
 * Method:    delete
 * Signature: (JJJ)V
 */
void Java_org_rocksdb_SstFileWriter_delete__J_3B(JNIEnv *env, jobject /*jobj*/,
                                                 jlong jhandle,
                                                 jbyteArray jkey) {
  jbyte *key = env->GetByteArrayElements(jkey, nullptr);
  if (key == nullptr) {
    // exception thrown: OutOfMemoryError
    return;
  }
  TERARKDB_NAMESPACE::Slice key_slice(reinterpret_cast<char *>(key),
                           env->GetArrayLength(jkey));

  TERARKDB_NAMESPACE::Status s =
      reinterpret_cast<TERARKDB_NAMESPACE::SstFileWriter *>(jhandle)->Delete(key_slice);

  env->ReleaseByteArrayElements(jkey, key, JNI_ABORT);

  if (!s.ok()) {
    TERARKDB_NAMESPACE::RocksDBExceptionJni::ThrowNew(env, s);
  }
}

/*
 * Class:     org_rocksdb_SstFileWriter
 * Method:    delete
 * Signature: (JJJ)V
 */
void Java_org_rocksdb_SstFileWriter_delete__JJ(JNIEnv *env, jobject /*jobj*/,
                                               jlong jhandle,
                                               jlong jkey_handle) {
  auto *key_slice = reinterpret_cast<TERARKDB_NAMESPACE::Slice *>(jkey_handle);
  TERARKDB_NAMESPACE::Status s =
      reinterpret_cast<TERARKDB_NAMESPACE::SstFileWriter *>(jhandle)->Delete(*key_slice);
  if (!s.ok()) {
    TERARKDB_NAMESPACE::RocksDBExceptionJni::ThrowNew(env, s);
  }
}

/*
 * Class:     org_rocksdb_SstFileWriter
 * Method:    finish
 * Signature: (J)V
 */
void Java_org_rocksdb_SstFileWriter_finish(JNIEnv *env, jobject /*jobj*/,
                                           jlong jhandle) {
  TERARKDB_NAMESPACE::Status s =
      reinterpret_cast<TERARKDB_NAMESPACE::SstFileWriter *>(jhandle)->Finish();
  if (!s.ok()) {
    TERARKDB_NAMESPACE::RocksDBExceptionJni::ThrowNew(env, s);
  }
}

/*
 * Class:     org_rocksdb_SstFileWriter
 * Method:    disposeInternal
 * Signature: (J)V
 */
void Java_org_rocksdb_SstFileWriter_disposeInternal(JNIEnv * /*env*/,
                                                    jobject /*jobj*/,
                                                    jlong jhandle) {
  delete reinterpret_cast<TERARKDB_NAMESPACE::SstFileWriter *>(jhandle);
}
