#include <iomanip>
#include <memory>
#include <iostream>
#include <span>
#include <ctime>
#include <libofx/libofx.h>

using OFXContextOwnerPtr = std::unique_ptr<void, int(*)(void*)>;

static constexpr const char* account_type_names[] = {
    /*OFX_CHECKING*/   "CHECKING",
    /*OFX_SAVINGS*/    "SAVINGS",
    /*OFX_MONEYMRKT*/  "MONEYMRKT",
    /*OFX_CREDITLINE*/ "CREDITLINE",
    /*OFX_CMA*/        "CMA",
    /*OFX_CREDITCARD*/ "CREDITCARD",
    /*OFX_INVESTMENT*/ "INVESTMENT",
    /*OFX_401K*/       "401K",
    /*Default*/        "Unknown"
};

static constexpr const char* transaction_type_names[] = {
    /*OFX_CREDIT*/      "CREDIT",
    /*OFX_DEBIT*/       "DEBIT",
    /*OFX_INT*/         "INTEREST",
    /*OFX_DIV*/         "DIVIDEND",
    /*OFX_FEE*/         "FEE",
    /*OFX_SRVCHG*/      "SERVICE CHARGE",
    /*OFX_DEP*/         "DEPOSIT",
    /*OFX_ATM*/         "ATM",
    /*OFX_POS*/         "POINT OF SALE",
    /*OFX_XFER*/        "TRANSFER",
    /*OFX_CHECK*/       "CHECK",
    /*OFX_PAYMENT*/     "PAYMENT",
    /*OFX_CASH*/        "CASH",
    /*OFX_DIRECTDEP*/   "DIRECT DEPOSIT",
    /*OFX_DIRECTDEBIT*/ "DIRECT DEBIT",
    /*OFX_REPEATPMT*/   "REPEAT PAYMENT",
    /*OFX_OTHER*/       "OTHER",
    /*Default*/         "Unknown"
};

template<typename T>
static
T try_lookup(std::span<const T> table, int enum_val)
{
    if(enum_val < 0 || (size_t)enum_val >= table.size()) {
        return table.back();
    }
    return table[enum_val];
}

static
auto to_date_time_str(time_t date)
{
    return std::put_time(std::localtime(&date), "%F");
}

#define print_field_value_if_valid(data, field_label, field, value) \
    if(data.field##_valid) { \
        std::cerr << field_label << ": " << value << "\n"; \
    }

#define print_if_valid(data, field_label, field) \
    print_field_value_if_valid(data, field_label, field, data.field)

#define print_date_if_valid(data, field_label, field) \
    print_field_value_if_valid(data, field_label, field, to_date_time_str(data.field))

int main(int argc, char** argv)
{
    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " path_to_ofx_file\n";
        return 1;
    }
    OFXContextOwnerPtr ctx{libofx_get_new_context(), libofx_free_context};
    ofx_set_account_cb(ctx.get(), [](const OfxAccountData data, void*) {
        std::cerr << "Account: " << data.account_name << "\n";
        if(data.account_type_valid) {
            std::cerr << "  Type: " << try_lookup<const char*>(account_type_names, data.account_type) << "\n";
        }
        return 1;
    }, nullptr);

    ofx_set_transaction_cb(ctx.get(), [](const OfxTransactionData data, void*) {
        if(!data.name_valid || !data.transactiontype_valid) {
            std::cerr << "Error: transaction type/name invalid\n";
            return 0;
        }
        std::cerr << "  Transaction: " << data.name << "\n"
                  << "    Type: " << try_lookup<const char*>(transaction_type_names, data.transactiontype) << "\n";
        print_date_if_valid(data, "    Date Initiated", date_initiated)
        print_date_if_valid(data, "    Date Posted", date_posted)
        print_date_if_valid(data, "    Date Purchased", date_purchase)
        print_date_if_valid(data, "    Date Funds Available", date_funds_available)
        print_date_if_valid(data, "    Date Payroll", date_payroll)

        print_if_valid(data, "    Amount", amount)
        return 1;
    }, nullptr);

    if(libofx_proc_file(ctx.get(), argv[1], LibofxFileFormat::AUTODETECT)) {
        std::cerr << "Error: failed to process OFX file]\n";
        return 1;
    }

    return 0;
}
