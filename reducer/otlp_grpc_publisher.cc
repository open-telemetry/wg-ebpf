// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <config.h>

#include "otlp_grpc_publisher.h"

#include <reducer/internal_metrics_encoder.h>
#include <reducer/internal_stats.h>
#include <util/time.h>

namespace reducer {

OtlpGrpcPublisher::OtlpGrpcPublisher(size_t num_writer_threads, const std::string &server_address_and_port)
    : server_address_and_port_(server_address_and_port)
{}

OtlpGrpcPublisher::~OtlpGrpcPublisher() {}

Publisher::WriterPtr OtlpGrpcPublisher::make_writer(size_t thread_num)
{
  return std::make_unique<Writer>(thread_num, server_address_and_port_);
}

void OtlpGrpcPublisher::write_internal_stats(InternalMetricsEncoder &encoder, u64 time_ns) const {}

////////////////////////////////////////////////////////////////////////////////
// Writer
//

OtlpGrpcPublisher::Writer::Writer(size_t thread_num, std::string const &server_address_and_port)
    : thread_num_(thread_num),
      server_address_and_port_(server_address_and_port),
      logs_client_(grpc::CreateChannel(server_address_and_port, grpc::InsecureChannelCredentials())),
      metrics_client_(grpc::CreateChannel(server_address_and_port, grpc::InsecureChannelCredentials()))
{}

OtlpGrpcPublisher::Writer::~Writer() {}

void OtlpGrpcPublisher::Writer::write(ExportLogsServiceRequest &request)
{
  logs_client_.AsyncExport(request);
}

void OtlpGrpcPublisher::Writer::write(ExportMetricsServiceRequest &request)
{
  metrics_client_.AsyncExport(request);
}

void OtlpGrpcPublisher::Writer::flush()
{
  logs_client_.process_async_responses();
  metrics_client_.process_async_responses();
}

void OtlpGrpcPublisher::Writer::write_internal_stats(
    InternalMetricsEncoder &encoder, u64 time_ns, int shard, std::string_view module) const
{
  OtlpGrpcStats stats;
  stats.labels.shard = std::to_string(shard);
  stats.labels.module = module;
  stats.metrics.bytes_failed = metrics_client_.bytes_failed();
  stats.metrics.bytes_sent = metrics_client_.bytes_sent();
  stats.metrics.metrics_failed = metrics_client_.entries_failed();
  stats.metrics.metrics_sent = metrics_client_.entries_sent();
  stats.metrics.requests_failed = metrics_client_.requests_failed();
  stats.metrics.requests_sent = metrics_client_.requests_sent();
  stats.metrics.unknown_response_tags = metrics_client_.unknown_response_tags();
  encoder.write_internal_stats(stats, time_ns);
}

} // namespace reducer
