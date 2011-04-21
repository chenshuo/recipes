package billing;

// FIXME: should be in groovy
public class DataFields {

    public static enum UserField {
        kUserType,
        kJoinTime,
        kIsNewUser,
        kPackages,
        kSlips,
        kDaysServed,
    }

    public static enum SlipType {
        kPhoneCall,
        kShortMessage,
        kInternet,
    }

    public static enum UserType {
        kNormal,
        kVip;

        public String getRuleName() {
            return name().substring(1).toLowerCase() + "_user";
        }
    }

    public static enum PackageType {
        kNormalUserPhoneCall,
        kNormalUserShortMessage,
        kNormalUserInternet,

        kVipUserPackage1,
        kVipUserPackage2,
    }
}
