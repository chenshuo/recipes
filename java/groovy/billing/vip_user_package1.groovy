package billing;

import static billing.DataFields.*;

public class VipUserPackage1Rule extends Rule {
    public static final long kPipsPackage = 100 * 10000L;
    public static final long kPipsPhoneCallPerMinute = 3000L;
    public static final long kNoChargeMinutes = 750;
    public static final long kPipsPerShortMessage = 1000L;
    public static final long kNoChargeShortMessages = 200;
    public static final long kPipsPerKiloBytes = 10;
    public static final long kNoChargeKiloBytes = 100*1000L;

    @Override
    long getMoneyInPips(UserMonthUsage input) {
        UserType type = (UserType)input.get(UserField.kUserType);
        assert type == UserType.kVip;
        List<Package> packages = input.getPackages();
        assert packages.get(0) == PackageType.kVipUserPackage1;

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
