#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import re

from ..base.file_base_check import FileBaseCheck
from ..system.registry import register_beman_standard_check

# [README.*] checks category.
# All checks in this file extend the ReadmeBaseCheck class.
#
# Note: ReadmeBaseCheck is not a registered check!


class ReadmeBaseCheck(FileBaseCheck):
    def __init__(self, repo_info, beman_standard_check_config):
        super().__init__(repo_info, beman_standard_check_config, "README.md")


@register_beman_standard_check("README.TITLE")
class ReadmeTitleCheck(ReadmeBaseCheck):
    def __init__(self, repo_info, beman_standard_check_config):
        super().__init__(repo_info, beman_standard_check_config)

    def check(self):
        lines = self.read_lines_strip()
        first_line = lines[0]

        # Match the pattern "# <self.library_name>[: <short_description>]"
        regex = rf"^# {re.escape(self.library_name)}: (.*)$"  # noqa: F541
        if not re.match(regex, first_line):
            self.log(
                f"The first line of the file '{self.path}' is invalid. It should start with '# {self.library_name}: <short_description>'."
            )
            return False

        return True

    def fix(self):
        """
        Fix the issue if the Beman Standard is not applied.
        """
        new_title_line = f"# {self.library_name}: TODO Short Description"
        self.replace_line(0, new_title_line)
        return True


@register_beman_standard_check("README.BADGES")
class ReadmeBadgesCheck(ReadmeBaseCheck):
    def __init__(self, repo_info, beman_standard_check_config):
        super().__init__(repo_info, beman_standard_check_config)

    def check(self):
        """
        self.config["values"] contains a fixed set of Beman badges.
        """
        badges = self.config["values"]
        assert len(badges) == 4  # The number of library maturity model states

        # Check if exactly one of the required badges is present.
        badge_count = len([badge for badge in badges if self.has_content(badge)])
        if badge_count != 1:
            self.log(
                f"The file '{self.path}' does not contain exactly one of the required badges from {badges}"
            )
            return False

        return True

    def fix(self):
        # TODO: Implement the fix.
        pass


@register_beman_standard_check("README.IMPLEMENTS")
class ReadmeImplementsCheck(ReadmeBaseCheck):
    def __init__(self, repo_info, beman_standard_check_config):
        super().__init__(repo_info, beman_standard_check_config)

    def check(self):
        lines = self.read_lines_strip()

        # Match the pattern to start with "Implements:" and then have a paper reference and a wg21.link URL.
        # Examples of valid lines:
        # **Implements**: [Standard Library Concepts (P0898R3)](https://wg21.link/P0898R3).
        # **Implements**: [Give *std::optional* Range Support (P3168R2)](https://wg21.link/P3168R2) and [`std::optional<T&>` (P2988R5)](https://wg21.link/P2988R5)
        # **Implements**: [.... (PxyzwRr)](https://wg21.link/PxyzwRr), [.... (PabcdRr)](https://wg21.link/PabcdRr), and [.... (PijklRr)](https://wg21.link/PijklRr),
        regex = r"^\*\*Implements\*\*:\s+.*\bP\d{4}R\d+\b.*wg21\.link/\S+"

        # Count how many lines match the regex
        implement_lines = 0
        for line in lines:
            if re.match(regex, line):
                implement_lines += 1

        # If there is exactly one "Implements:" line, it is valid
        if implement_lines == 1:
            return True

        # Invalid/missing/duplicate "Implements:" line
        self.log(
            f"Invalid/missing/duplicate 'Implements:' line in '{self.path}'. See https://github.com/bemanproject/beman/blob/main/docs/BEMAN_STANDARD.md#readmeimplements for more information."
        )
        return False

    def fix(self):
        self.log(
            "Please write a Implements line in README.md file. See https://github.com/bemanproject/beman/blob/main/docs/BEMAN_STANDARD.md#readmeimplements for the desired format."
        )
        return False


@register_beman_standard_check("README.LIBRARY_STATUS")
class ReadmeLibraryStatusCheck(ReadmeBaseCheck):
    def __init__(self, repo_info, beman_standard_check_config):
        super().__init__(repo_info, beman_standard_check_config)

    def check(self):
        """
        self.config["values"] contains a fixed set of Beman library statuses.
        """
        statuses = self.config["values"]
        assert len(statuses) == len(self.beman_library_maturity_model)

        # Check if at least one of the required status values is present.
        status_count = len([status for status in statuses if self.has_content(status)])
        if status_count != 1:
            self.log(
                f"The file '{self.path}' does not contain exactly one of the required statuses from {statuses}"
            )
            return False

        return True

    def fix(self):
        # TODO: Implement the fix.
        pass
