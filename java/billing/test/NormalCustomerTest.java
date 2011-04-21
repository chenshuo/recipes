package billing.test;

import static org.junit.Assert.assertEquals;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.joda.time.LocalDateTime;
import org.junit.Test;

import billing.BillCalculator;
import billing.Money;
import billing.Slip;
import billing.UserMonthUsage;
import billing.DataFields.PackageType;
import billing.DataFields.SlipType;
import billing.DataFields.UserField;
import billing.DataFields.UserType;

public class NormalCustomerTest {

    private static final BillCalculator calculator = new BillCalculator("./groovy/billing/");

    @Test
    public void testNormalUserNoPackageNoActivity() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kNormal);
        UserMonthUsage usage = new UserMonthUsage(1, data);
        assertEquals(new Money(0), calculator.calculate(usage));
    }

    @Test
    public void testNormalUserNoPackageOneCall() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kNormal);
        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kPhoneCall, new LocalDateTime(), 1));
        data.put(UserField.kSlips, slips);
        UserMonthUsage usage = new UserMonthUsage(2, data);
        assertEquals(Money.get(0, 60), calculator.calculate(usage));
    }

    @Test
    public void testNormalUserNoPackageOneShortMessage() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kNormal);
        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kShortMessage, new LocalDateTime(), 1));
        data.put(UserField.kSlips, slips);
        UserMonthUsage usage = new UserMonthUsage(3, data);
        assertEquals(Money.get(0, 10), calculator.calculate(usage));
    }

    @Test
    public void testNormalUserNoPackage100kData() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kNormal);
        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kInternet, new LocalDateTime(), 100));
        data.put(UserField.kSlips, slips);
        UserMonthUsage usage = new UserMonthUsage(4, data);
        assertEquals(Money.get(0, 50), calculator.calculate(usage));
    }

    public List<PackageType> getPackages(PackageType... packages) {
        return Arrays.asList(packages);
    }

    @Test
    public void testNormalUserPhoneCallPackageNoActivity() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kNormal);

        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kPhoneCall, new LocalDateTime(), 0));
        data.put(UserField.kSlips, slips);

        data.put(UserField.kPackages, getPackages(PackageType.kNormalUserPhoneCall));

        UserMonthUsage usage = new UserMonthUsage(5, data);
        assertEquals(Money.get(20, 0), calculator.calculate(usage));
    }

    @Test
    public void testNormalUserPhoneCallPackageWithinLimit() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kNormal);

        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kPhoneCall, new LocalDateTime(), 60));
        data.put(UserField.kSlips, slips);

        data.put(UserField.kPackages, getPackages(PackageType.kNormalUserPhoneCall));

        UserMonthUsage usage = new UserMonthUsage(5, data);
        assertEquals(Money.get(20, 0), calculator.calculate(usage));
    }

    @Test
    public void testNormalUserPhoneCallPackageOneMoreCall() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kNormal);

        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kPhoneCall, new LocalDateTime(), 61));
        data.put(UserField.kSlips, slips);

        data.put(UserField.kPackages, getPackages(PackageType.kNormalUserPhoneCall));

        UserMonthUsage usage = new UserMonthUsage(5, data);
        assertEquals(Money.get(20, 50), calculator.calculate(usage));
    }

    @Test
    public void testNormalUserShortMessagePackageWithinLimit() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kNormal);

        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kShortMessage, new LocalDateTime(), 200));
        data.put(UserField.kSlips, slips);

        data.put(UserField.kPackages, getPackages(PackageType.kNormalUserShortMessage));

        UserMonthUsage usage = new UserMonthUsage(5, data);
        assertEquals(Money.get(10, 0), calculator.calculate(usage));
    }

    @Test
    public void testNormalUserShortMessagePackageOneMore() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kNormal);

        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kShortMessage, new LocalDateTime(), 201));
        data.put(UserField.kSlips, slips);

        data.put(UserField.kPackages, getPackages(PackageType.kNormalUserShortMessage));

        UserMonthUsage usage = new UserMonthUsage(5, data);
        assertEquals(Money.get(10, 10), calculator.calculate(usage));
    }

    @Test
    public void testNormalUserInternetPackageWithinLimit() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kNormal);

        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kInternet, new LocalDateTime(), 50 * 1000));
        data.put(UserField.kSlips, slips);

        data.put(UserField.kPackages, getPackages(PackageType.kNormalUserInternet));

        UserMonthUsage usage = new UserMonthUsage(5, data);
        assertEquals(Money.get(20, 0), calculator.calculate(usage));
    }

    @Test
    public void testNormalUserInternetPackage50kMore() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kNormal);

        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kInternet, new LocalDateTime(), 50 * 1000 + 50));
        data.put(UserField.kSlips, slips);

        data.put(UserField.kPackages, getPackages(PackageType.kNormalUserInternet));

        UserMonthUsage usage = new UserMonthUsage(5, data);
        assertEquals(Money.get(20, 15), calculator.calculate(usage));
    }

    @Test
    public void testNormalUserAllPackagesNoActivity() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kNormal);

        List<Slip> slips = new ArrayList<Slip>();
        // slips.add(new Slip(SlipType.kPhoneCall, new LocalDateTime(), 0));
        data.put(UserField.kSlips, slips);

        data.put(UserField.kPackages, getPackages(PackageType.kNormalUserPhoneCall,
                PackageType.kNormalUserShortMessage, PackageType.kNormalUserInternet));

        UserMonthUsage usage = new UserMonthUsage(5, data);
        assertEquals(Money.get(50, 0), calculator.calculate(usage));
    }

    @Test
    public void testNormalUserAllPackagesActivitiesWithInLimit() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kNormal);

        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kPhoneCall, new LocalDateTime(), 60));
        slips.add(new Slip(SlipType.kShortMessage, new LocalDateTime(), 200));
        slips.add(new Slip(SlipType.kInternet, new LocalDateTime(), 50 * 1000));
        data.put(UserField.kSlips, slips);

        data.put(UserField.kPackages, getPackages(PackageType.kNormalUserPhoneCall,
                PackageType.kNormalUserShortMessage, PackageType.kNormalUserInternet));

        UserMonthUsage usage = new UserMonthUsage(5, data);
        assertEquals(Money.get(50, 0), calculator.calculate(usage));
    }

    @Test
    public void testNormalUserAllPackagesActivitiesExceedInLimit() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kNormal);

        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kPhoneCall, new LocalDateTime(), 61));
        slips.add(new Slip(SlipType.kShortMessage, new LocalDateTime(), 210));
        slips.add(new Slip(SlipType.kInternet, new LocalDateTime(), 50 * 1000 + 50));
        data.put(UserField.kSlips, slips);

        data.put(UserField.kPackages, getPackages(PackageType.kNormalUserPhoneCall,
                PackageType.kNormalUserShortMessage, PackageType.kNormalUserInternet));

        UserMonthUsage usage = new UserMonthUsage(5, data);
        assertEquals(Money.get(51, 65), calculator.calculate(usage));
    }

    @Test
    public void testNormalUserNewJoinWithInLimit() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kNormal);

        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kPhoneCall, new LocalDateTime(), 60));
        slips.add(new Slip(SlipType.kShortMessage, new LocalDateTime(), 200));
        slips.add(new Slip(SlipType.kInternet, new LocalDateTime(), 50 * 1000));
        data.put(UserField.kSlips, slips);

        data.put(UserField.kIsNewUser, true);

        UserMonthUsage usage = new UserMonthUsage(5, data);
        assertEquals(new Money(0), calculator.calculate(usage));
    }
}
