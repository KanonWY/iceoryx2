// Copyright (c) 2024 Contributors to the Eclipse Foundation
//
// See the NOTICE file(s) distributed with this work for additional
// information regarding copyright ownership.
//
// This program and the accompanying materials are made available under the
// terms of the Apache Software License 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0, or the MIT license
// which is available at https://opensource.org/licenses/MIT.
//
// SPDX-License-Identifier: Apache-2.0 OR MIT

#include "iox2/node_name.hpp"
#include "iox/assertions.hpp"
#include "iox/into.hpp"

#include <cstring>

namespace iox2 {
NodeName::NodeName(iox2_node_name_h handle)
    : m_handle { handle } {
}

NodeName::~NodeName() {
    drop();
}

NodeName::NodeName(NodeName&& rhs) noexcept
    : m_handle { std::move(rhs.m_handle) } {
    rhs.m_handle = nullptr;
}

auto NodeName::operator=(NodeName&& rhs) noexcept -> NodeName& {
    if (this != &rhs) {
        drop();
        m_handle = std::move(rhs.m_handle);
        rhs.m_handle = nullptr;
    }

    return *this;
}

NodeName::NodeName(const NodeName& rhs)
    : m_handle { nullptr } {
    auto value = rhs.to_string();
    IOX_ASSERT(iox2_node_name_new(nullptr, value.c_str(), value.size(), &m_handle) == IOX2_OK,
               "NodeName shall always contain a valid value.");
}

auto NodeName::operator=(const NodeName& rhs) -> NodeName& {
    if (this != &rhs) {
        drop();

        auto value = rhs.to_string();
        IOX_ASSERT(iox2_node_name_new(nullptr, value.c_str(), value.size(), &m_handle) == IOX2_OK,
                   "NodeName shall always contain a valid value.");
    }

    return *this;
}

void NodeName::drop() noexcept {
    if (m_handle != nullptr) {
        iox2_node_name_drop(m_handle);
        m_handle = nullptr;
    }
}

auto NodeName::create(const char* value) -> iox::expected<NodeName, SemanticStringError> {
    iox2_node_name_h handle {};
    const auto value_len = strnlen(value, NODE_NAME_LENGTH + 1);
    if (value_len == NODE_NAME_LENGTH + 1) {
        return iox::err(SemanticStringError::ExceedsMaximumLength);
    }

    const auto ret_val = iox2_node_name_new(nullptr, value, value_len, &handle);
    if (ret_val == IOX2_OK) {
        return iox::ok(std::move(NodeName { handle }));
    }

    return iox::err(iox::from<iox2_semantic_string_error_e, SemanticStringError>(
        static_cast<iox2_semantic_string_error_e>(ret_val)));
}

auto NodeName::to_string() const -> iox::string<NODE_NAME_LENGTH> {
    const auto* ptr = iox2_cast_node_name_ptr(m_handle);
    size_t len = 0;
    const auto* c_ptr = iox2_node_name_as_c_str(ptr, &len);
    return { iox::TruncateToCapacity, c_ptr, len };
}

} // namespace iox2
