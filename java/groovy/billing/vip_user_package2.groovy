package billing;

import static billing.DataFields.*;

public class VipUserPackage2Rule extends Rule {
    public static final long kPipsPackage = 200 * 10000L;
    public static final long kPipsPhoneCallPerMinute = 2000L;
    public static final long kNoChargeMinutes = 2000;
    public static final long kPipsPerShortMessage = 1000L;
    public static final long kNoChargeShortMessages = 500;
    public static final long kPipsPerKiloBytes = 5;
    public static final long kNoChargeKiloBytes = 300*1000L;

    @Override
    long getMoneyInPips(UserMonthUsage input) {
        UserType type = (UserType)input.get(UserField.kUserType);
        assert type == UserType.kVip;
        List<Package> packages = input.getPackages();
        assert packages.get(0) == PackageType.kVipUserPackage2;

        //int dayServed = input.getInteger(UserField.kDaysServed);
        Rule phoneCall = factory.create("package_phone_call",
             [ 0L, kNoChargeMinutes, kPipsPhoneCallPerMinute ]);
        Rule shortMessage = factory.create("package_short_message",
             [ 0L, kNoChargeShortMessages, kPipsPerShortMessage ]);
        Rule internet = factory.create("package_internet",
             [ 0L, kNoChargeKiloBytes, kPipsPerKiloBytes ]);
        long total = phoneCall.getMoneyInPips(input) +
                     shortMessage.getMoneyInPips(input) +
                     internet.getMoneyInPips(input) +
                     kPipsPackage;
        return total;
    }
}
