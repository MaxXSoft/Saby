#ifndef SABY_UTIL_DATA_EXPORTER_EXPORTER_H_
#define SABY_UTIL_DATA_EXPORTER_EXPORTER_H_

#include <memory>
#include <utility>
#include <ostream>
#include <string>
#include <list>

class DataElementInterface;
class DataReference;
class ExporterInterface;

using UIDType = unsigned long long;
using DataElement = std::unique_ptr<DataElementInterface>;
using DataGroup = std::list<DataElement>;
using DataRefGroup = std::list<DataReference>;
using Exporter = std::unique_ptr<ExporterInterface>;

class DataElementInterface {
public:
    virtual ~DataElementInterface() = default;

    virtual void AddData(const std::string &key, long long value) = 0;
    void AddData(const std::string &key, char value) { AddData(key, static_cast<long long>(value)); }
    void AddData(const std::string &key, int value) { AddData(key, static_cast<long long>(value)); }
    void AddData(const std::string &key, short value) { AddData(key, static_cast<long long>(value)); }
    void AddData(const std::string &key, long value) { AddData(key, static_cast<long long>(value)); }
    virtual void AddData(const std::string &key, unsigned long long value) = 0;
    void AddData(const std::string &key, unsigned char value) { AddData(key, static_cast<unsigned long long>(value)); }
    void AddData(const std::string &key, unsigned int value) { AddData(key, static_cast<unsigned long long>(value)); }
    void AddData(const std::string &key, unsigned short value) { AddData(key, static_cast<unsigned long long>(value)); }
    void AddData(const std::string &key, unsigned long value) { AddData(key, static_cast<unsigned long long>(value)); }
    virtual void AddData(const std::string &key, double value) = 0;
    void AddData(const std::string &key, float value) { AddData(key, static_cast<double>(value)); }
    virtual void AddData(const std::string &key, bool value) = 0;
    virtual void AddData(const std::string &key, const std::string &value) = 0;
    virtual void AddData(const std::string &key, std::nullptr_t) = 0;
    virtual void AddData(const std::string &key, DataElement &&value) = 0;
    virtual void AddData(const std::string &key, DataReference &&value) = 0;
    virtual void AddData(const std::string &key, DataGroup &&value) = 0;
    virtual void AddData(const std::string &key, DataRefGroup &&value) = 0;

    virtual UIDType uid() const = 0;
};

class DataReference {
public:
    explicit DataReference(DataElement &data) : data_(data) {}

    DataElement &data() { return data_; }
    UIDType uid() const { return data_->uid(); }

private:
    DataElement &data_;
};

class ExporterInterface {
public:
    virtual ~ExporterInterface() = default;

    virtual DataElement &NewDataElement(const std::string &name) = 0;
    virtual DataGroup &NewDataGroup(const std::string &name) = 0;
    virtual DataRefGroup &NewDataRefGroup(const std::string &name) = 0;
    virtual void Export(std::ostream &&os) = 0;
    void Export(std::ostream &os) { Export(std::move(os)); }

    virtual void set_version(int major, int minor, int revision) = 0;
};

#endif // SABY_UTIL_DATA_EXPORTER_EXPORTER_H_
