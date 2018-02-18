#ifndef SABY_DATA_EXPORTER_EXPORTER_H_
#define SABY_DATA_EXPORTER_EXPORTER_H_

#include <memory>
#include <utility>
#include <ostream>
#include <string>
#include <list>

class DataElementInterface;
class DataReference;
class Exporter;

using UIDType = unsigned long long;
using DataElement = std::unique_ptr<DataElementInterface>;
using DataGroup = std::list<DataElement>;
using DataRefGroup = std::list<DataReference>;
using Exporter = std::unique_ptr<Exporter>;

class DataElementInterface {
public:
    virtual ~DataElementInterface() = default;

    virtual void AddData(const std::string &key, long long value) = 0;
    virtual void AddData(const std::string &key, double value) = 0;
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

#endif // SABY_DATA_EXPORTER_EXPORTER_H_
