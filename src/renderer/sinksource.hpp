#pragma once
#include <string>
#include <typeindex>

#include "renderer/IncompatibleLinkageException.hpp"
#include "renderer/RenderPass.hpp"

namespace sge {

// might require forward declaration

template <typename T>
struct sink;

class RenderPass;

struct isource {
  friend struct isink;

 public:
  explicit isource(const std::string& name, const std::type_index typeIndex)
      : m_name(name), m_typeIndex(typeIndex) {}

 public:
  const std::string m_name;
  RenderPass* m_pass = nullptr;

 private:
  const std::type_index m_typeIndex;
};

template <typename T>
struct source : public isource {
  friend struct sink<T>;

 public:
  explicit source(const std::string name, T* p_value = nullptr)
      : isource(name, std::type_index(typeid(T))), m_ptr(p_value) {}

  void set(T* p_value) { m_ptr = p_value; }
  void set(sink<T>& sink) { m_ptr = &(*sink); }
  void reset() { m_ptr = nullptr; }

 private:
  T* m_ptr;
};

struct isink {
 public:
  explicit isink(const std::string& name, const std::type_index typeIndex,
                 boolean nullable)
      : m_name(name), m_typeIndex(typeIndex), m_nullable(nullable) {}
  void link(isource* p_source) {
    assert(p_source != nullptr);
    if (p_source->m_typeIndex != m_typeIndex) {
      throw IncompatibleLinkageException();
    }
    linkInternal(p_source);
  }

 protected:
  virtual void linkInternal(isource* p_source) = 0;

 public:
  const std::string m_name;
  RenderPass* m_pass = nullptr;
  boolean m_nullable = false;

 private:
  const std::type_index m_typeIndex;
};

template <typename T>
struct sink : public isink {
 public:
  explicit sink(const std::string name, boolean nullable = false)
      : isink(name, std::type_index(typeid(T)), nullable) {}

  T& operator*() const { return *(m_ptr->m_ptr); }
  T* operator->() {
    if (m_nullable && m_ptr == nullptr) return nullptr;
    return m_ptr->m_ptr;
  }
  operator bool() const {
    if (m_ptr == nullptr) return false;
    return m_ptr->m_ptr != nullptr;
  }

 protected:
  void linkInternal(isource* p_source) override {
    m_ptr = reinterpret_cast<source<T>*>(p_source);
  }

 protected:
  boolean m_isFwd = false;

 private:
  source<T>* m_ptr;
};

struct isinksource {
  virtual isink* getSink() = 0;
  virtual isource* getSource() = 0;
};

template <typename T>
struct sinksource : public isinksource {
 public:
  explicit sinksource(const std::string& name)
      : m_sink(sink<T>(name)), m_source(source<T>(name)) {}

  T& operator*() const { return *m_sink; }
  T* operator->() { return m_sink; }
  operator bool() const { return (bool)m_sink; }
  void forward() { m_source.set(m_sink); }
  void set(T* p_value) { m_source.set(p_value); }
  void set(sink<T>& sink) { m_source.set(sink); }
  void reset() { m_source.reset(); }
  isink* getSink() { return &m_sink; }
  isource* getSource() { return &m_source; }

 private:
  sink<T> m_sink;
  source<T> m_source;
};

}  // namespace sge
