#ifndef SABY_DATA_EXPORTER_EXPORTER_H_
#define SABY_DATA_EXPORTER_EXPORTER_H_

#include <memory>
#include <string>
#include <list>
#include <cstddef>

class DataElementInterface;
class DataReference;
class Exporter;

using UIDType = std::size_t;
using DataGroup = std::list<std::unique_ptr<DataElementInterface>>;
using DataRefGroup = std::list<std::unique_ptr<DataReference>>;
using Exporter = std::unique_ptr<Exporter>;

class DataElementInterface {
public:
    virtual ~DataElementInterface() = default;

    virtual void SetData(const std::string &key, int value) = 0;
    virtual void SetData(const std::string &key, bool value) = 0;
    virtual void SetData(const std::string &key, const std::string &value) = 0;
    virtual void SetData(const std::string &key, std::nullptr_t) = 0;
    virtual void SetData(const std::string &key, DataRefGroup &&ref) = 0;

    virtual void set_uid(UIDType uid) = 0;
    virtual void set_type(int type) = 0;

    virtual UIDType uid() const = 0;
    virtual int type() const = 0;
};

class DataReference {
public:
    explicit DataReference(DataElementInterface &data) : data_(data) {}

    DataElementInterface &data() { return data_; }
    UIDType uid() const { return data_.uid(); }

private:
    DataElementInterface &data_;
};

class ExporterInterface {
public:
    virtual ~ExporterInterface() = default;

    virtual DataGroup &NewDataGroup(const std::string &name) = 0;
    virtual DataRefGroup &NewDataRefGroup(const std::string &name) = 0;
    virtual void Export(const std::string &path) = 0;

    virtual void set_version(int major, int minor, int revision) = 0;

protected:
    //
};

#endif // SABY_DATA_EXPORTER_EXPORTER_H_
