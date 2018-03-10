#include "def_use.h"

void Value::ReplaceBy(const SSAPtr &value) {
    // TODO: consider optimizing
    // copy an use list from current value
    auto uses = uses_;
    // reroute all uses to new value
    for (const auto &use : uses) {
        use->set_value(value);
    }
}

void Use::set_value(const SSAPtr &value) {
    assert(copied_);
    if (value_) value_->RemoveUse(this);
    value_ = value;
    if (value_) value_->AddUse(this);
}
