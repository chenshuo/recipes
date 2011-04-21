package billing;

import static billing.DataFields.*;

public class NormalUserPhoneCallRule extends Rule {

    public static final long kPipsPhoneCallPerMinute = 6000L;

    public static final long kPipsPhoneCallPerMinuteWithPackage = 5000L;
    public static final long kPipsPhoneCallPackage = 20*10000L;
    public static final long kNoChargeMinutesInPackage = 60L;

    @Override
    long getMoneyInPips(UserMonthUsage input) {
        UserType type = (UserType)input.get(UserField.kUserType);
        assert type == UserType.kNormal;

        List<Package> packages = input.getPackages();

        if (packages.contains(PackageType.kNormalUserPhoneCall)) {
            boolean isNew = input.getBoolean(UserField.kIsNewUser);
            assert !isNew;
            Rule phoneCall = factory.create("package_phone_call",
                 [ kPipsPhoneCallPackage, kNoChargeMinutesInPackage, kPipsPhoneCallPerMinuteWithPackage ]);
            return phoneCall.getMoneyInPips(input);
        } else {
            long noChargeMinutes = (Long)state;
            Rule phoneCall = factory.create("package_phone_call",
                 [ 0L, noChargeMinutes, kPipsPhoneCallPerMinute ]);
            return phoneCall.getMoneyInPips(input);
        }
    }
}
