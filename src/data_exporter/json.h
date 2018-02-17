#ifndef SABY_DATA_EXPORTER_JSON_H_
#define SABY_DATA_EXPORTER_JSON_H_

#include "exporter.h"

#include <map>

class JSONExporter : public ExporterInterface {
public:
    explicit JSONExporter() {}
    ~JSONExporter() {}

    DataElement &NewDataElement(const std::string &name) { return elements_[name]; }
    DataGroup &NewDataGroup(const std::string &name) { return groups_[name]; }
    DataRefGroup &NewDataRefGroup(const std::string &name) { return ref_groups_[name]; }
    void Export(std::ostream &&os) override;

    void set_version(int major, int minor, int revision) {
        ver_major_ = major;
        ver_minor_ = minor;
        ver_revision_ = revision;
    }

private:
    int ver_major_, ver_minor_, ver_revision_;
    std::map<std::string, DataElement> elements_;
    std::map<std::string, DataGroup> groups_;
    std::map<std::string, DataRefGroup> ref_groups_;
};

class JSONDataElement : public DataElementInterface {
public:
    explicit JSONDataElement(UIDType uid) : uid_(uid) {}
    explicit JSONDataElement() : uid_(reinterpret_cast<UIDType>(this)) {}
    ~JSONDataElement() {}

    void AddData(const std::string &key, long long value) override;
    void AddData(const std::string &key, double value) override;
    void AddData(const std::string &key, bool value) override;
    void AddData(const std::string &key, const std::string &value) override;
    void AddData(const std::string &key, std::nullptr_t) override;
    void AddData(const std::string &key, DataGroup &&value) override;
    void AddData(const std::string &key, DataRefGroup &&value) override;

    UIDType uid() const override { return uid_; }
    const std::string &data_str() const { return data_str_; }

private:
    UIDType uid_;
    std::string data_str_;
};

#endif // SABY_DATA_EXPORTER_EXPORTER_H_